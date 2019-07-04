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

#include "3dparty/nlohmann/json.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#define KShimDataDef "KShimData"

using namespace std;
using namespace nlohmann;

namespace {
static const int DataStorageSize = 1024 * 2;

// don't make const to prevent optimisation
struct command
{
    char cmd[DataStorageSize];
};

static command StartupCommand { KShimDataDef };
}

KShimData::KShimData()
    : m_rawData(StartupCommand.cmd, StartupCommand.cmd + sizeof(StartupCommand.cmd))
{
    if (isShim()) {
        kLog << "Load raw Data: " << m_rawData.data();
        json data = json::parse(m_rawData.data());
        m_app = data["app"].get<KShim::string>();
        m_args = data["args"].get<vector<KShim::string>>();
        m_env = data["env"].get<vector<pair<KShim::string, KShim::string>>>();
    }
}

KShim::path KShimData::app() const
{
    return m_app;
}

KShim::path KShimData::appAbs() const
{
    return makeAbsouteCommand(app());
}

void KShimData::setApp(const KShim::path &app)
{
    m_app = app;
}

const vector<KShim::string> &KShimData::args() const
{
    return m_args;
}

void KShimData::setArgs(const vector<KShim::string> &args)
{
    m_args = args;
}

void KShimData::addArg(const KShim::string &arg)
{
    m_args.push_back(arg);
}

KShim::string KShimData::formatCommand(const vector<KShim::string> &arguments) const
{
    KShim::stringstream cmd;
    cmd << quote(appAbs()) << formatArgs(arguments);
    return cmd.str();
}

KShim::string KShimData::formatArgs(const std::vector<KShim::string> &arguments) const
{
    KShim::stringstream cmd;
    cmd << quoteArgs(args()) << quoteArgs(arguments);
    return cmd.str();
}

const vector<char> &KShimData::rawData() const
{
    return m_rawData;
}

std::string KShimData::toJson() const
{
    auto out =
            json {
#ifdef _WIN32
                { "app", app().wstring() },
#else
                { "app", app().string() },
#endif
                { "args", args() },
                { "env", env() },
            }
                    .dump();
    kLog << "toJson:" << out.data();
    return out;
}

bool KShimData::isShim() const
{
    kLog << "isShim: " << m_rawData.data();
    return m_rawData.data() != std::string(KShimDataDef);
}

KShim::path KShimData::makeAbsouteCommand(const KShim::path &path) const
{
    if (path.is_absolute()) {
        return path;
    } else {
        return KShim::binaryName().parent_path() / path;
    }
}

vector<pair<KShim::string, KShim::string>> KShimData::env() const
{
    return m_env;
}

void KShimData::setEnv(const vector<pair<KShim::string, KShim::string>> &env)
{
    m_env = env;
}

void KShimData::addEnvVar(const pair<KShim::string, KShim::string> &var)
{
    m_env.push_back(var);
}

KShim::string KShimData::quote(const KShim::string &arg) const
{
    // based on https://github.com/python/cpython/blob/master/Lib/subprocess.py#L493
    bool needsQuote = false;
    for (const auto c : arg) {
        needsQuote = c == ' ' || c == '\t';
        if (needsQuote) {
            break;
        }
    }
    KShim::stringstream out;
    KShim::stringstream backslash;
    if (needsQuote) {
        out << '"';
    }
    for (const auto c : arg) {
        if (c == '\\') {
            backslash << c;
        } else if (c == '"') {
            const auto bs = backslash.str();
            out << bs << bs << "\\\"";
            backslash.str(KShim::string());
        } else {
            const auto bs = backslash.str();
            if (!bs.empty()) {
                out << bs;
                backslash.str(KShim::string());
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

KShim::string KShimData::quoteArgs(vector<KShim::string> args) const
{
    KShim::stringstream command;
    for (const auto &arg : args) {
        command << " " << quote(arg);
    }
    return command.str();
}
