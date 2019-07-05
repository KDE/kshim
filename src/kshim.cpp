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

#include "kshim.h"
#include "kshimpath.h"
#include "kshimdata.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#ifndef _WIN32
#include <sys/stat.h>
#else
#include <windows.h>
#endif

using namespace std;

namespace {

KShim::path normaliseApplicationName(const KShim::path &app)
{
#ifdef _WIN32
    KShim::path out = app;
    out.replace_extension(KSTRING_LITERAL(".exe"));
    return out;
#else
    return app;
#endif
}

KShim::path shimName(bool createGuiApplication)
{

    const auto path = KShim::binaryName().parent_path();
    KShim::path name = KSTRING_LITERAL("kshim");
#ifdef _WIN32
    if (createGuiApplication)
    {
        name = KSTRING_LITERAL("kshimgui");
    }
#endif
    return path / normaliseApplicationName(name);
}


vector<char> readBinary(bool createGuiApplication)
{
    const auto name = shimName(createGuiApplication);
#if defined(KSHIM_HAS_FILESYSTEM) || !defined(__MINGW32__)
    const auto &_name = name;
#else
    const auto _name = name.string();
#endif
    ifstream me(_name, ios::in | ios::binary);
    if (!me.is_open()) {
        kLog2(KLog::Type::Error) << "Failed to open: " << name;
        return {};
    }

    me.seekg(0, me.end);
    size_t size = static_cast<size_t>(me.tellg());
    me.seekg(0, me.beg);

    vector<char> buf(size);
    me.read(buf.data(), static_cast<streamsize>(size));
    me.close();
    kLog << "Read: " << name << " " << size << " bytes";
    return buf;
}

bool writeBinary(const KShim::path &name, const KShimData &shimData, const vector<char> &binary)
{
    vector<char> dataOut = binary;

    // look for the end mark and search for the start from there
    const auto &rawData = shimData.rawData();
    const auto cmdIt = search(dataOut.begin(), dataOut.end(), rawData.cbegin(), rawData.cend());
    if (cmdIt == dataOut.end()) {
        kLog2(KLog::Type::Error) << "Failed to patch binary, please report your compiler";
        exit(1);
    }

#if defined(KSHIM_HAS_FILESYSTEM) || !defined(__MINGW32__)
    const auto &_name = name;
#else
    const auto _name = name.string();
#endif
    ofstream out(_name, ios::out | ios::binary);
    if (!out.is_open()) {
        kLog2(KLog::Type::Error) << "Failed to open out: " << name;
        return false;
    }
    const std::string json = shimData.toJson();
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

bool KShim::createShim(const KShim::string &appName, const KShim::path &target,
                       const vector<KShim::string> &args, const vector<KShim::string> &_env, bool createGuiApplication)
{    
    vector<pair<KShim::string, KShim::string>> env;
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

bool KLog::s_loggingEnabled = !KShim::getenv(KSTRING_LITERAL("KSHIM_LOG")).empty();

KLog::KLog(KLog::Type t) : m_type(t), m_stream(new KShim::stringstream) {}

KLog::KLog(const KLog &other) : m_type(other.m_type), m_stream(other.m_stream) {}

KLog::~KLog()
{
    if (m_stream.use_count() == 1) {
        *this << "\n";
        const auto line = m_stream->str();
        switch (m_type) {
        case KLog::Type::Error:
#ifdef _WIN32
            std::wcerr << line;
#else
            std::cerr << line;
#endif
        case KLog::Type::Debug: {
            if (doLog()) {
                static auto _log = [] {
                    auto home = KShim::getenv(KSTRING_LITERAL("HOME"));
                    if (home.empty()) {
                        home = KShim::getenv(KSTRING_LITERAL("USERPROFILE"));
                    }
                    const auto logPath = KShim::path(home) / KSTRING_LITERAL(".kshim.log");
#ifdef _WIN32
#if defined(KSHIM_HAS_FILESYSTEM) || !defined(__MINGW32__)
                    const auto &_name = logPath;
#else
                    const auto _name = logPath.string();
#endif
                    auto out = std::wofstream(_name, std::ios::app);
#else
                    auto out = std::ofstream(logPath, std::ios::app);
#endif
                    if (!out.is_open()) {
                        std::cerr << "KShim: Failed to open log \"" << logPath.string() << "\" "
                                  << strerror(errno) << std::endl;
                    }
                    out << "----------------------------\n";
                    return out;
                }();
#ifdef _WIN32
                OutputDebugStringW(line.data());
#endif
                _log << line;
            }
        }
        }
    }
}

KLog &KLog::log()
{
    *this << "KShimgen " << KShim::version << ": ";
    return *this;
}

KLog::Type KLog::type() const
{
    return m_type;
}

bool KLog::doLog() const
{
    return loggingEnabled() || m_type != KLog::Type::Debug;
}

bool KLog::loggingEnabled()
{
    return s_loggingEnabled;
}

void KLog::setLoggingEnabled(bool loggingEnabled)
{
    s_loggingEnabled = loggingEnabled;
}

int KShim::shimgen_main(const std::vector<KShim::string> &args)
{
    KShim::string target;
    KShim::string app;
    vector<KShim::string> arguments;
    vector<KShim::string> env;
    bool gui = false;

    auto help = [](const KShim::string &msg) {
        kLog2(KLog::Type::Error)
                << msg << "\n"
                << "--create shim target\t\t\tCreate a shim\n"
                << "--env key=val\t\t\t\tadditional environment varriables for the shim\n"
           #ifdef _WIN32
                << "--gui\t\t\t\t\tcreate a gui application (only supported on Windows)\n"
           #endif
                << "-- arg1 arg2 arg3...\t\t\targuments that get passed to the target";
    };
    auto nextArg = [&](std::vector<KShim::string>::const_iterator &it,
            const KShim::string &helpText) -> KShim::string {
        if (it != args.cend()) {
            return *it++;
        } else {
            help(helpText);
            exit(1);
        }
    };

    auto it = args.cbegin() + 1;
    while (it != args.cend()) {
        const auto arg = nextArg(it, KSTRING_LITERAL(""));
        if (arg == KSTRING_LITERAL("--create")) {
            const auto msg = KSTRING_LITERAL("--create shim target");
            app = nextArg(it, msg);
            target = nextArg(it, msg);
        } else if (arg == KSTRING_LITERAL("--env")) {
            env.push_back(nextArg(it, KSTRING_LITERAL("--env key=val")));
        } else if (arg == KSTRING_LITERAL("--")) {
            while (it != args.cend()) {
                arguments.push_back(nextArg(it, KSTRING_LITERAL("")));
            }
            break;
#ifdef _WIN32
        } else if(arg == KSTRING_LITERAL("--gui")) {
            gui = true;
#endif
        } else if (arg == KSTRING_LITERAL("-h")) {
            help(KSTRING_LITERAL(""));
        } else {
            KShim::stringstream str;
            str << "Unknwon arg " << arg;
            help(str.str());
        }
    }
    if (!target.empty()) {
        return KShim::createShim(app, target, arguments, env, gui) ? 0 : -1;
    }
    help(KSTRING_LITERAL(""));
    return -1;
}

int KShim::shim_main(const std::vector<KShim::string> &args)
{
    KShimData data;
    if (!data.isShim()) {
        kLog2(KLog::Type::Error) << "Please call kshimgen to generate the shim";
    } else {
        const auto tmp = std::vector<KShim::string>(args.cbegin() + 1, args.cend());
        int out = KShim::run(data, tmp);
        kLog << "Exit: " << out;
        return out;
    }
    return -1;
}

KLog &operator<<(KLog &log, const KShim::path &t)
{
#ifdef _WIN32
    log << t.wstring();
#else
    log << t.string();
#endif
    return log;
}

KLog &operator<<(KLog &log, const string &t)
{
    log << t.data();
    return log;
}
