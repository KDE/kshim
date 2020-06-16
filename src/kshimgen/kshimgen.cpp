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

#include <algorithm>

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(KShimEmbeddeResource);

using namespace std;

namespace {

KShimLib::path normaliseApplicationName(const KShimLib::path &app)
{
#ifdef _WIN32
    KShimLib::path out = app;
    out.replace_extension(KShimLib::exeSuffixW);
    return out;
#else
    return app;
#endif
}

vector<char> readBinary(bool createGuiApplication)
{
    std::string name = "bin/kshim"s + std::string(KShimLib::exeSuffix);
#ifdef _WIN32
    if (createGuiApplication) {
        name = "bin/kshimgui"s + std::string(KShimLib::exeSuffix);
    }
#endif
    const auto filesystem = cmrc::KShimEmbeddeResource::get_filesystem();
    const auto binary = filesystem.open(name);
    return vector<char>(binary.begin(), binary.end());
}

bool writeBinary(const KShimLib::path &name, const KShimData &shimData, const vector<char> &binary)
{
    vector<char> dataOut = binary;

    // look for the end mark and search for the start from there
    const std::string marker(KShimDataDef);
    vector<char> rawData(KShimLib::DataStorageSize, 0);
    std::copy(marker.cbegin(), marker.cend(), rawData.begin());
    const auto cmdIt = search(dataOut.begin(), dataOut.end(), rawData.cbegin(), rawData.cend());
    if (cmdIt == dataOut.end()) {
        kLog2(KLog::Type::Error) << "Failed to patch binary, please report your compiler";
        exit(1);
    }

#if KSHIM_HAS_FILESYSTEM
    const auto &_name = name;
#elif defined(__MINGW32__)
    const auto _name = name.string();
#else
    const auto _name = KShimLib::string(name);
#endif
    ofstream out(_name, ios::out | ios::binary);
    if (!out.is_open()) {
        kLog2(KLog::Type::Error) << "Failed to open out: " << name;
        return false;
    }
    const std::vector<uint8_t> json = shimData.toJson();
    if (json.size() > rawData.size()) {
        kLog2(KLog::Type::Error) << "Data buffer is too small " << json.size() << " > "
                                 << rawData.size() << " :\n"
                                 << json.data();
        return false;
    }
    copy(json.cbegin(), json.cend(), cmdIt);
    out.write(dataOut.data(), static_cast<streamsize>(binary.size()));
    kLog << "Wrote: " << name << " " << out.tellp() << " bytes";
    out.close();

#ifndef _WIN32
    chmod(name.string().data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return true;
}
}

bool KShimGen::createShim(const KShimLib::string_view &appName, const KShimLib::path &target,
                          const vector<KShimLib::string_view> &args,
                          const vector<KShimLib::string_view> &_env, bool createGuiApplication)
{
    vector<pair<KShimLib::string_view, KShimLib::string_view>> env;
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
    const vector<char> binary = readBinary(createGuiApplication);
    if (!binary.empty()) {
        return writeBinary(outApp, shimData, binary);
    }
    return false;
}

int KShimGen::main(const std::vector<KShimLib::string_view> &args)
{
    KShimLib::string_view target;
    KShimLib::string_view app;
    vector<KShimLib::string_view> arguments;
    vector<KShimLib::string_view> env;
    bool gui = false;

    auto help =
            [](const KShimLib::string_view &msg) {
                kLog2(KLog::Type::Error)
                        << msg << "\n"
                        << "--create shim target\t\t\tCreate a shim\n"
                        << "--env key=val\t\t\t\tadditional environment varriables for the shim\n"
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
    if (!target.empty()) {
        return KShimGen::createShim(app, target, arguments, env, gui) ? 0 : -1;
    }
    help(KSTRING(""));
    return -1;
}
