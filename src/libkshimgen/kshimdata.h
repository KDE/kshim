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

#ifndef KSHIMDATA_H
#define KSHIMDATA_H

#include "kshim.h"

#include <string>
#include <vector>

#ifndef WIN32
// don't make const to prevent optimisation
struct KShimPayLoad
{
    // an initialised data array starts with an size_t with the size of the payload
    // followed by the formated payload
    size_t size;
    uint8_t cmd[KShimLib::DataStorageSize];
};
#endif

class KShimData
{
public:
    KShimData() = default;
    KShimData(const std::filesystem::path &app);
    KShimData(const std::vector<uint8_t> &payLoad);

    std::filesystem::path app() const;
    std::filesystem::path appAbs() const;
    void setApp(const std::filesystem::path &app);

    /***
     * If isEnvOverrideEnabled is true this will return either appAbs or the absolute KSHIM_shimname
     * env var
     */
    std::filesystem::path appAbsWithOverride() const;

    const std::vector<KShimLib::string> &args() const;
    void setArgs(const std::vector<KShimLib::string_view> &args);
    void addArg(const KShimLib::string_view &arg);

    const std::vector<std::pair<KShimLib::string, KShimLib::string>> &env() const;
    void setEnv(const std::vector<std::pair<KShimLib::string_view, KShimLib::string_view>> &env);
    void addEnvVar(const std::pair<KShimLib::string_view, KShimLib::string_view> &var);

    KShimLib::string formatArgs(const std::vector<KShimLib::string_view> &args) const;

    /***
     *
     * When enabled the shim will try look for KSHIM_shimname for app()
     */
    bool isEnvOverrideEnabled() const;
    void setEnvOverrideEnabled(bool envOverrideEnabled);

    /***
     * Pass the original arg0 to the target process to emulate symlink behaviour
     */
    bool isKeepArgv0Enabled() const;
    void setKeepArgv0Enabled(bool keepArg0Enabled);

    std::vector<uint8_t> toJson() const;

private:
    std::filesystem::path makeAbsouteCommand(const std::filesystem::path &_path) const;

    std::filesystem::path m_app;
    std::vector<KShimLib::string> m_args;
    std::vector<std::pair<KShimLib::string, KShimLib::string>> m_env;
    bool m_envOverrideEnabled = false;
    bool m_keepArgv0Enabled = false;
};

#endif // KSHIMDATA_H
