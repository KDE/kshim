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
#include "kshimpath.h"

#include <string>
#include <vector>

class KShimData
{
public:
    KShimData();
    KShimData(const std::vector<char> &data);

    KShimLib::path app() const;
    KShimLib::path appAbs() const;
    void setApp(const KShimLib::path &app);

    const std::vector<KShimLib::string> &args() const;
    void setArgs(const std::vector<KShimLib::string> &args);
    void addArg(const KShimLib::string &arg);

    std::vector<std::pair<KShimLib::string, KShimLib::string>> env() const;
    void setEnv(const std::vector<std::pair<KShimLib::string, KShimLib::string>> &env);
    void addEnvVar(const std::pair<KShimLib::string, KShimLib::string> &var);

    KShimLib::string formatCommand(const std::vector<KShimLib::string> &args) const;
    KShimLib::string formatArgs(const std::vector<KShimLib::string> &args) const;

    std::vector<uint8_t> toJson() const;

private:
    KShimLib::string quote(const KShimLib::string &arg) const;
    KShimLib::string quoteArgs(std::vector<KShimLib::string> args) const;
    KShimLib::path makeAbsouteCommand(const KShimLib::path &_path) const;

    KShimLib::path m_app;
    std::vector<KShimLib::string> m_args;
    std::vector<std::pair<KShimLib::string, KShimLib::string>> m_env;
    const std::vector<char> m_rawData;
};

#endif // KSHIMDATA_H
