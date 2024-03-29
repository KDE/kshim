/*
    Copyright Hannah von Reth <vonreth@kde.org>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
#include "kshimgen.h"
#include "kshimdata.h"
#include "kshimgen_p.h"

#include <algorithm>
#include <cstring>

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(KShimEmbeddeResource);

namespace {

std::filesystem::path normaliseApplicationName(const std::filesystem::path &app)
{
#ifdef _WIN32
    std::filesystem::path out = app;
    out.replace_extension(KShimLib::exeSuffixW);
    return out;
#else
    return app;
#endif
}

std::vector<char> readBinary(bool createGuiApplication)
{
    std::string name = "bin/kshim"s + std::string(KShimLib::exeSuffix);
#ifdef _WIN32
    if (createGuiApplication) {
        name = "bin/kshimgui"s + std::string(KShimLib::exeSuffix);
    }
#else
    (void)createGuiApplication;
#endif
    const auto filesystem = cmrc::KShimEmbeddeResource::get_filesystem();
    const auto binary = filesystem.open(name);
    return std::vector<char>(binary.begin(), binary.end());
}

bool writeBinary(const std::filesystem::path &name, const KShimData &shimData,
                 std::vector<char> &&dataOut)
{
#ifndef _WIN32
    KShimGenPrivate::patchBinary(dataOut, shimData.toJson());
#endif
    const auto &_name = name;
    {
        std::ofstream out(_name, std::ios::out | std::ios::binary);
        if (!out.is_open()) {
            kLog2(KLog::Type::Error) << "Failed to open out: " << name;
            return false;
        }
        out.write(dataOut.data(), static_cast<std::streamsize>(dataOut.size()));

        kLog << "Wrote: " << name << " " << out.tellp() << " bytes";
        out.close();
    }

#ifndef _WIN32
    chmod(name.string().data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#ifdef __APPLE__
    const auto codesign = KShimLib::findInPath(std::filesystem::path(KShimLib::string("codesign")));
    const int result = KShimLib::run(KShimData(codesign), { "-s", "-", name.string() });
    if (result != 0) {
        kLog << "Faied to sign" << name.string() << "exit code:" << result;
        return false;
    }
#endif
#else
    KShimGenPrivate::setPayload(name, shimData.toJson());
    // we can't use shimData.appAbs() as it depends on the location of the current binary, which
    // atm is kshimgen
    auto src = shimData.app();
    if (!src.is_absolute()) {
        src = name.parent_path() / shimData.app();
    }
    if (!KShimLib::exists(src)) {
        src = KShimLib::findInPath(shimData.app());
    }
    KShimGenPrivate::updateIcon(src, name);
#endif
    return true;
}
}

bool KShimGen::createShim(const KShimLib::string_view &appName, const std::filesystem::path &target,
                          const std::vector<KShimLib::string_view> &args,
                          const std::vector<KShimLib::string_view> &_env, bool createGuiApplication,
                          bool enableEnvOverride, bool keepArg0)
{
    std::vector<std::pair<KShimLib::string_view, KShimLib::string_view>> env;
    env.reserve(_env.size());
    for (const auto &e : _env) {
        const auto pos = e.find('=');
        env.push_back({ e.substr(0, pos), e.substr(pos + 1) });
    }
    const auto outApp = normaliseApplicationName(appName);
    KShimData shimData;
    shimData.setApp(target);
    shimData.setArgs(args);
    shimData.setEnv(env);
    shimData.setEnvOverrideEnabled(enableEnvOverride);
    shimData.setKeepArgv0Enabled(keepArg0);
    std::vector<char> binary = readBinary(createGuiApplication);
    if (!binary.empty()) {
        return writeBinary(outApp, shimData, std::move(binary));
    }
    return false;
}

int KShimGen::main(const std::vector<KShimLib::string_view> &args)
{
    KShimLib::string_view target;
    KShimLib::string_view app;
    std::vector<KShimLib::string_view> arguments;
    std::vector<KShimLib::string_view> env;
    bool gui = false;
    bool enableEnvOverride = false;
    bool keepArgv0 = false;

    auto help = [](const KShimLib::string_view &msg) {
        kLog2(KLog::Type::Error)
                << msg << "\n"
                << "--create shim target\t\t\tCreate a shim\n"
                << "--env key=val\t\t\t\tadditional environment varriables for the shim\n"
                << "--enable-env-override\t\t\twhether to allow overriding the target with "
                   "the env var KSHIM_shim\n"
                << "--keep-argv0\t\t\t\twhether to keep the original arg0 (emulates symlinks)\n"
#ifdef _WIN32
                << "--gui\t\t\t\t\tcreate a gui application (only supported on Windows)\n"
#endif
                << "-- arg1 arg2 arg3...\t\t\targuments that get passed to the target";
    };
    auto nextArg = [&](std::vector<KShimLib::string_view>::const_iterator &it,
                       const KShimLib::string_view &helpText) -> KShimLib::string_view {
        if (it != args.cend()) {
            return *it++;
        } else {
            help(helpText);
            exit(1);
        }
    };

    auto it = args.cbegin() + 1;
    while (it != args.cend()) {
        const auto arg = nextArg(it, KSTRING(""));
        if (arg == KSTRING("--create")) {
            const auto msg = KSTRING("--create shim target");
            app = nextArg(it, msg);
            target = nextArg(it, msg);
        } else if (arg == KSTRING("--env")) {
            env.push_back(nextArg(it, KSTRING("--env key=val")));
        } else if (arg == KSTRING("--enable-env-override")) {
            enableEnvOverride = true;
        } else if (arg == KSTRING("--keep-argv0")) {
            keepArgv0 = true;
        } else if (arg == KSTRING("--")) {
            while (it != args.cend()) {
                arguments.push_back(nextArg(it, KSTRING("")));
            }
            break;
#ifdef _WIN32
        } else if (arg == KSTRING("--gui")) {
            gui = true;
#endif
        } else if (arg == KSTRING("-h")) {
            help(KSTRING(""));
            return 0;
        } else {
            KShimLib::stringstream str;
            str << "Unknwon arg " << arg;
            help(str.str());
        }
    }
    if (enableEnvOverride) {
        if (!env.empty()) {
            help(KSTRING("--enable-env-override and --env are mutual exclusive"s));
        }
        if (!arguments.empty()) {
            help(KSTRING(
                    "When using --enable-env-override it is not supported to provide extra args after --"s));
        }
    }
    if (!target.empty()) {
        return KShimGen::createShim(app, target, arguments, env, gui, enableEnvOverride, keepArgv0)
                ? 0
                : -1;
    }
    help(KSTRING(""));
    return -1;
}
