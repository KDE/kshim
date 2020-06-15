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

#include "../3dparty/nlohmann/json.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;
using namespace nlohmann;

namespace {
enum class Format { Json, Ubjson };
Format KSHIM_DATA_FORMAT()
{
    static Format format = [] {
        const KShimLib::string var = KShimLib::getenv(KSTRING_LITERAL("KSHIM_FORMAT"), KSTRING_LITERAL("ubjson"));
        kLog << "DataFormat: " << var;
        if (var == KSTRING_LITERAL("ubjson")) {
            return Format::Ubjson;
        }
        return Format::Json;
    }();
    return format;
}
}

KShimData::KShimData() {}

KShimData::KShimData(const vector<char> &rawData) : m_rawData(rawData)
{
    json data;
    switch (KSHIM_DATA_FORMAT()) {
    case Format::Json:
        data = json::parse(m_rawData.data());
        break;
    case Format::Ubjson:
        data = json::from_ubjson(m_rawData.data());
        break;
    }
    kLog << "Json Data: " << data.dump(4);
    m_app = data["app"].get<KShimLib::string>();
    m_args = data["args"].get<vector<KShimLib::string>>();
    m_env = data["env"].get<vector<pair<KShimLib::string, KShimLib::string>>>();
}

KShimLib::path KShimData::app() const
{
    return m_app;
}

KShimLib::path KShimData::appAbs() const
{
    return makeAbsouteCommand(app());
}

void KShimData::setApp(const KShimLib::path &app)
{
    m_app = app;
}

const vector<KShimLib::string> &KShimData::args() const
{
    return m_args;
}

void KShimData::setArgs(const vector<KShimLib::string> &args)
{
    m_args = args;
}

void KShimData::addArg(const KShimLib::string &arg)
{
    m_args.push_back(arg);
}

KShimLib::string KShimData::formatCommand(const vector<KShimLib::string> &arguments) const
{
    KShimLib::stringstream cmd;
    cmd << quote(appAbs()) << formatArgs(arguments);
    return cmd.str();
}

KShimLib::string KShimData::formatArgs(const std::vector<KShimLib::string> &arguments) const
{
    KShimLib::stringstream cmd;
    cmd << quoteArgs(args()) << quoteArgs(arguments);
    return cmd.str();
}

std::vector<uint8_t> KShimData::toJson() const
{
    const auto jsonData = json {
#ifdef _WIN32
        { "app", app().wstring() },
#else
        { "app", app().string() },
#endif
        { "args", args() },
        { "env", env() },
    };
    std::vector<uint8_t> out;
    switch (KSHIM_DATA_FORMAT()) {
    case Format::Json: {
        const auto dump = jsonData.dump();
        out = std::vector<uint8_t>(dump.cbegin(), dump.cend());
        break;
    }
    case Format::Ubjson:
        out = json::to_ubjson(jsonData);
    }

    kLog << "toJson:" << out.size();
    return out;
}

KShimLib::path KShimData::makeAbsouteCommand(const KShimLib::path &path) const
{
    if (path.is_absolute()) {
        return path;
    } else {
        return KShimLib::binaryName().parent_path() / path;
    }
}

vector<pair<KShimLib::string, KShimLib::string>> KShimData::env() const
{
    return m_env;
}

void KShimData::setEnv(const vector<pair<KShimLib::string, KShimLib::string>> &env)
{
    m_env = env;
}

void KShimData::addEnvVar(const pair<KShimLib::string, KShimLib::string> &var)
{
    m_env.push_back(var);
}

KShimLib::string KShimData::quote(const KShimLib::string &arg) const
{
    // based on https://github.com/python/cpython/blob/master/Lib/subprocess.py#L493
    bool needsQuote = false;
    for (const auto c : arg) {
        needsQuote = c == ' ' || c == '\t';
        if (needsQuote) {
            break;
        }
    }
    KShimLib::stringstream out;
    KShimLib::stringstream backslash;
    if (needsQuote) {
        out << '"';
    }
    for (const auto c : arg) {
        if (c == '\\') {
            backslash << c;
        } else if (c == '"') {
            const auto bs = backslash.str();
            out << bs << bs << "\\\"";
            backslash.str(KShimLib::string());
        } else {
            const auto bs = backslash.str();
            if (!bs.empty()) {
                out << bs;
                backslash.str(KShimLib::string());
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

KShimLib::string KShimData::quoteArgs(vector<KShimLib::string> args) const
{
    KShimLib::stringstream command;
    for (const auto &arg : args) {
        command << " " << quote(arg);
    }
    return command.str();
}
