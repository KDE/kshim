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

#include "kshimdata.h"
#include "kshim.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#define KShimDataDef "KShimData"

using namespace std;
using namespace nlohmann;

namespace  {
static const int DataStorageSize = 1024 * 2;

struct command
{
#if defined(_MSC_VER) && _MSC_VER <= 1900
    // don't make const to prevent optimisation
    char cmd[DataStorageSize];
#else
    const char cmd[DataStorageSize];
#endif
};

static const command StartupCommand {
    KShimDataDef
};
}

KShimData::KShimData()
    : m_rawData(StartupCommand.cmd, StartupCommand.cmd +  sizeof(StartupCommand.cmd))
{
    if (isShim())
    {
        kLog << "Load raw Data: " << m_rawData.data();
        json data = json::parse(m_rawData.data());
        m_app = data["app"].get<string>();
        m_args = data["args"].get<vector<string>>();
        m_env = data["env"].get<vector<string>>();
    }
}

string KShimData::app() const
{
    return m_app;
}

void KShimData::setApp(const string &app)
{
    m_app = app;
}

const vector<string> &KShimData::args() const
{
    return m_args;
}

void KShimData::setArgs(const vector<string> &args)
{
    m_args = args;
}

void KShimData::addArg(const string &arg)
{
    m_args.push_back(arg);
}

string KShimData::formatCommand(const vector<string> &arguments) const
{
#ifdef _WIN32
    const string extraQuote = "\"";
#else
    const string extraQuote = "";
#endif
    stringstream cmd;
    cmd << extraQuote
        << makeAbsouteCommand(app())
        << quoteArgs(args())
        << quoteArgs(arguments)
        << extraQuote;
    return cmd.str();
}

const vector<char> &KShimData::rawData() const
{
    return m_rawData;
}

string KShimData::toJson() const
{
    auto out = json{
    {"app", app()},
    {"args", args()},
    {"env", env()},
}.dump();
    kLog << "toJson:" << out;
    return out;
}

bool KShimData::isShim() const
{
    return m_rawData.data() != string(KShimDataDef);
}


string KShimData::makeAbsouteCommand(const string &_path) const
{
    string path = _path;
    stringstream out;
    if (path[0] == '"')
    {
        out << '"';
        path = path.erase(0, 1);
    }
    if (path[0] != '.') {
        out <<  path;
    } else {
        auto app = KShim::binaryName();
        app = app.substr(0, app.rfind(KShim::dirSep()));
        out << app << KShim::dirSep() << path;
    }
    return quote(out.str());
}

std::vector<std::string> KShimData::env() const
{
    return m_env;
}

void KShimData::setEnv(const std::vector<std::string> &env)
{
    m_env = env;
}

void KShimData::addEnvVar(const string &var)
{
    m_env.push_back(var);
}

string KShimData::quote(const string &arg) const
{
    // based on https://github.com/python/cpython/blob/master/Lib/subprocess.py#L493
    bool needsQuote = true;
#if 0
    // TODO: why do we need to quote everything?
    bool needsQuote = false;
    for (const auto c : arg) {
        needsQuote = c == ' ' || c == '\t';
        if (needsQuote) {
            break;
        }
    }
#endif
    stringstream out;
    stringstream backslash;
    if (needsQuote) {
        out << '"';
    }
    for (const auto c : arg)
    {
        if(c == '\\') {
            backslash << c;
        } else if (c == '"') {
            const auto bs = backslash.str();
            out << bs << bs << "\\\"";
            backslash.str(std::string());
        } else {
            const auto bs = backslash.str();
            if (!bs.empty()) {
                out << bs;
                backslash.str(std::string());
            }
            out << c;
        }
    }
    const auto bs = backslash.str();
    if (!bs.empty()) {
        out << bs;
    }
    if (needsQuote) {
        out << bs;
        out << '"';
    }
    return out.str();
}

string KShimData::quoteArgs(vector<string> args) const
{
    stringstream command;
    for (const auto &arg : args) {
        command << " " << quote(arg);
    }
    return command.str();
}
