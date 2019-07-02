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

namespace  {

std::filesystem::path normaliseApplicationName(const std::filesystem::path &app)
{
#ifdef _WIN32
    std::filesystem::path out = app;
    out.replace_extension(".exe");
    return out;
#else
    return app;
#endif
}

vector<char> readBinary()
{
    const auto name = KShim::binaryName();
    ifstream me;
    me.open(name, ios::in | ios::binary);
    if (!me.is_open()) {
        cerr << "Failed to open: " << name << endl;
        return {};
    }

    me.seekg (0, me.end);
    size_t size = static_cast<size_t>(me.tellg());
    me.seekg (0, me.beg);

    vector<char> buf(size);
    me.read(buf.data(), static_cast<streamsize>(size));
    me.close();
    kLog << "Read: " << name << " " << size << " bytes";
    return buf;
}



bool writeBinary(const std::filesystem::path &name, const KShimData &shimData, const vector<char> &binary)
{
    vector<char> dataOut = binary;

    // look for the end mark and search for the start from there
    const auto &rawData = shimData.rawData();
    const auto cmdIt = search(dataOut.begin(), dataOut.end(), rawData.cbegin(), rawData.cend());
    if (cmdIt == dataOut.end()) {
        kLog2(KLog::Type::Error) << "Failed to patch binary, please report your compiler";
        exit(1);
    }

    ofstream out;
    out.open(name, ios::out | ios::binary);
    if (!out.is_open()) {
        kLog2(KLog::Type::Error) << "Failed to open out: " << name;
        return false;
    }
    const std::string json = shimData.toJson();
    if (json.size() > rawData.size()) {
        kLog2(KLog::Type::Error) << "Data buffer is too small " << json.size() << " > " << rawData.size() << " :\n" << json.data();
        return false;
    }
    copy(json.cbegin(), json.cend(), cmdIt);
    out.write(dataOut.data(), static_cast<streamsize>(binary.size()));
    kLog << "Wrote: " << name << " " << out.tellp() << " bytes";
    out.close();

#ifndef _WIN32
    chmod(name.string().data(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return true;
}
}

bool KShim::createShim(KShimData &shimData, const KShim::string &appName, const filesystem::path &target, const vector<KShim::string> &args,  const vector<KShim::string> &_env)
{
    vector<pair<KShim::string,KShim::string>> env;
    env.reserve(_env.size());
    for (const auto &e : _env)
    {
        const auto pos = e.find('=');
        env.push_back({e.substr(0, pos), e.substr(pos + 1)});
    }
    const auto outApp = normaliseApplicationName(appName);
    shimData.setApp(target);
    shimData.setArgs(args);
    shimData.setEnv(env);
    const vector<char> binary = readBinary();
    if (!binary.empty()) {
        return writeBinary(outApp, shimData, binary);
    }
    return false;
}

KLog::KLog(KLog::Type t)
    : m_type(t)
    , m_stream(new KShim::stringstream)
{
}

KLog::KLog(const KLog &other)
    : m_type(other.m_type)
    , m_stream(m_stream)
{

}

KLog::~KLog()
{
    if (m_stream.unique())
    {
        *this << "\n";
        const auto line = m_stream->str();
        switch(m_type)
        {
        case KLog::Type::Error:
#ifdef _WIN32
            std::wcerr << line;
#else
            std::cerr << line;
#endif
        case KLog::Type::Debug:
        {
            if (doLog())
            {
                static auto _log = []{
                    auto home = KShim::getenv(KSTRING("HOME"));
                    if (home.empty())
                    {
                        home = KShim::getenv(KSTRING("USERPROFILE"));
                    }
                    const auto logPath = std::filesystem::path(home) / KSTRING(".kshim.log");
#ifdef _WIN32
                    auto out = std::wofstream(logPath, std::wofstream::app);
#else
                    auto out = std::ofstream(logPath, std::wofstream::app);
#endif
                    if (!out.is_open())
                    {
                        cerr << "KShim: Failed to open log \"" << logPath.string() << "\" " << strerror(errno) << endl;
                    }
                    out << "----------------------------\n";
                    return out;
                }();
#ifdef _WIN32
                OutputDebugStringW(line.data());
#endif
                _log <<  line;
            }
        }
        }
    }
}

KLog &KLog::log() {
    *this << "KShimgen: ";
    return  *this;
}

KLog::Type KLog::type() const
{
    return m_type;
}

bool KLog::doLog() const
{
    static bool _do_log = !KShim::getenv(KSTRING("KSHIM_LOG")).empty();
    return _do_log || m_type != KLog::Type::Debug;
}

int KShim::main(const std::vector<KShim::string> &args)
{
    KShimData data;
    if (!data.isShim()) {
        KShim::string target;
        KShim::string app;
        vector<KShim::string> arguments;
        vector<KShim::string> env;

        auto nextArg = [&](auto &it) -> KShim::string {
            if (it != args.cend()) {
                return *it++;
            } else {
                //                help(helpText);
                exit(1);
            }
        };

        auto it = args.begin() + 1;
        while (it != args.end()) {
            const auto arg = nextArg(it);
            kLog << arg;

            if (arg == KSTRING("--create")){
                app = nextArg(it);
                target = nextArg(it);
            }
            else if (KSTRING("env") )
            {
                env.push_back(nextArg(it));
            }
            else
            {
                arguments.push_back(arg);
            }
        }
        if (!target.empty())
        {
            return KShim::createShim(data, app, target, arguments, env) ? 0 : -1;
        }
    } else {
        const auto tmp = std::vector<KShim::string>(args.cbegin()+1, args.cend());
        int out = KShim::run(data, tmp);
        kLog << "Exit: " << out;
        return out;
    }
    return -1;
}


