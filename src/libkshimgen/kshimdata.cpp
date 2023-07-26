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

KShimData::KShimData(const std::filesystem::path &app) : m_app(app) { }

KShimData::KShimData(const std::vector<uint8_t> &payLoad)
{
    kLog << "PayLoad Size: " << payLoad.size();
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
    m_app = std::filesystem::path(data["app"].get<KShimLib::string>());
    m_args = data["args"].get<std::vector<KShimLib::string>>();
    m_env = data["env"].get<std::vector<std::pair<KShimLib::string, KShimLib::string>>>();
    m_envOverrideEnabled = data["envOverrideEnabled"].get<bool>();
    m_keepArgv0Enabled = data["keepArgv0Enabled"].get<bool>();
}

std::filesystem::path KShimData::app() const
{
    return m_app;
}

std::filesystem::path KShimData::appAbs() const
{
    return makeAbsouteCommand(app());
}

void KShimData::setApp(const std::filesystem::path &app)
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

KShimLib::string KShimData::formatArgs(const std::vector<KShimLib::string_view> &arguments) const
{
    KShimLib::stringstream cmd;
    cmd << KShimLib::quoteArgs({ args().cbegin(), args().cend() })
        << KShimLib::quoteArgs(arguments);
    return cmd.str();
}

std::vector<uint8_t> KShimData::toJson() const
{
    const auto jsonData = json { { "app", app().native() },
                                 { "args", args() },
                                 { "env", env() },
                                 { "envOverrideEnabled", isEnvOverrideEnabled() },
                                 { "keepArgv0Enabled", isKeepArgv0Enabled() } };
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

std::filesystem::path KShimData::makeAbsouteCommand(const std::filesystem::path &path) const
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

bool KShimData::isEnvOverrideEnabled() const
{
    return m_envOverrideEnabled;
}

void KShimData::setEnvOverrideEnabled(bool envOverrideEnabled)
{
    m_envOverrideEnabled = envOverrideEnabled;
}

std::filesystem::path KShimData::appAbsWithOverride() const
{
    if (m_envOverrideEnabled) {
        KShimLib::stringstream stream;
        stream << "KSHIM_" << KShimLib::string(KShimLib::binaryName().filename());
        const auto overrideApp =
                std::filesystem::path(KShimLib::getenv(stream.str(), KShimLib::string(appAbs())));
        if (overrideApp.is_absolute()) {
            return overrideApp;
        }
        kLog2(KLog::Type::Error) << stream.str()
                                 << " must be absolute, current value: " << overrideApp
                                 << " falling back to internal value " << appAbs();
    }
    return appAbs();
}

bool KShimData::isKeepArgv0Enabled() const
{
    return m_keepArgv0Enabled;
}

void KShimData::setKeepArgv0Enabled(bool keepArgv0Enabled)
{
    m_keepArgv0Enabled = keepArgv0Enabled;
}
