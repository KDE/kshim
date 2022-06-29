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

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace nlohmann;

namespace {
enum class Format { Json, Ubjson };
Format KSHIM_DATA_FORMAT()
{
    static Format format = [] {
        const auto var = KShimLib::getenv(KSTRING("KSHIM_FORMAT"), KSTRING("ubjson"));
        kLog << "DataFormat: " << var;
        if (var == KSTRING("ubjson")) {
            return Format::Ubjson;
        }
        return Format::Json;
    }();
    return format;
}
}

KShimData::KShimData() {}

KShimData::KShimData(const PayLoad &payLoad)
{
    kLog << "PayLoad Size: " << payLoad.size;
    json data;
    switch (KSHIM_DATA_FORMAT()) {
    case Format::Json:
        data = json::parse(payLoad.cbegin(), payLoad.cend());
        break;
    case Format::Ubjson:
        data = json::from_ubjson(payLoad.cbegin(), payLoad.cend());
        break;
    }
    kLog << "Json Data: " << data.dump(4);
    m_app = KShimLib::path(data["app"].get<KShimLib::string>());
    m_shellArg = data["shellArg"].get<KShimLib::string>();
    m_args = data["args"].get<std::vector<KShimLib::string>>();
    m_env = data["env"].get<std::vector<std::pair<KShimLib::string, KShimLib::string>>>();
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

const std::vector<KShimLib::string> &KShimData::args() const
{
    return m_args;
}

void KShimData::setArgs(const std::vector<KShimLib::string_view> &args)
{
    m_args = { args.cbegin(), args.cend() };
}

void KShimData::addArg(const KShimLib::string_view &arg)
{
    m_args.push_back(KShimLib::string(arg));
}

void KShimData::setShellMode(const KShimLib::string_view &shellArg)
{
    m_shellArg = shellArg;
}

KShimLib::string KShimData::shellMode() const
{
    return m_shellArg;
}

bool KShimData::isShellModeEnabled() const
{
    return !m_shellArg.empty();
}

KShimLib::string KShimData::formatCommand(const std::vector<KShimLib::string_view> &arguments) const
{
    KShimLib::stringstream cmd;
    cmd <<
#ifdef _WIN32
            KShimLib::quote(appAbs().wstring())
#else
            quote(appAbs().string())
#endif
        << formatArgs(arguments);
    return cmd.str();
}

KShimLib::string KShimData::formatArgs(const std::vector<KShimLib::string_view> &arguments) const
{
    if (!args().empty()) {
        KShimLib::stringstream cmd;
        cmd << quoteArgs({ args().cbegin(), args().cend() }) << " " << quoteArgs(arguments);
        return cmd.str();
    } else {
        return quoteArgs(arguments);
    }
}

std::vector<uint8_t> KShimData::toJson() const
{
    const auto jsonData = json {
#ifdef _WIN32
        { "app", app().wstring() },
#else
        { "app", app().string() },
#endif
        { "shellArg", shellMode() },
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
        auto tmp = KShimLib::binaryName().parent_path() / path;
        if (tmp != KShimLib::binaryName() && KShimLib::exists(tmp)) {
            return tmp;
        }
    }
    return KShimLib::findInPath(path);
}

const std::vector<std::pair<KShimLib::string, KShimLib::string>> &KShimData::env() const
{
    return m_env;
}

void KShimData::setEnv(
        const std::vector<std::pair<KShimLib::string_view, KShimLib::string_view>> &env)
{
    m_env = { env.cbegin(), env.cend() };
}

void KShimData::addEnvVar(const std::pair<KShimLib::string_view, KShimLib::string_view> &var)
{
    m_env.push_back({ KShimLib::string(var.first), KShimLib::string(var.second) });
}

KShimLib::string KShimData::quote(const KShimLib::string_view &arg) const
{
    // based on https://github.com/python/cpython/blob/master/Lib/subprocess.py#L493
    if (arg.empty()) {
        return KSTRING("\"\"");
    }
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

KShimLib::string KShimData::quoteArgs(const std::vector<KShimLib::string_view> &args) const
{
    KShimLib::stringstream command;
    auto it = args.cbegin();
    command << *it++;
    for (; it != args.cend(); ++it) {
        command << " " << quote(*it);
    }
    return command.str();
}
