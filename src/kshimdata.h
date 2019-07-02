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
#include <filesystem>

class KShimData
{
public:
    KShimData();

    std::filesystem::path app() const;
    std::filesystem::path appAbs() const;
    void setApp(const std::filesystem::path &app);

    const std::vector<KShim::string> &args() const;
    void setArgs(const std::vector<KShim::string> &args);
    void addArg(const KShim::string &arg);

    std::vector<std::pair<KShim::string, KShim::string>> env() const;
    void setEnv(const std::vector<std::pair<KShim::string, KShim::string> > &env);
    void addEnvVar(const std::pair<KShim::string, KShim::string> &var);

    KShim::string formatCommand(const std::vector<KShim::string> &args) const;
    KShim::string formatArgs(const std::vector<KShim::string> &args) const;

    bool isShim() const;
    const std::vector<char> &rawData() const;

    std::string toJson() const;

private:
    KShim::string quote(const KShim::string &arg) const;
    KShim::string quoteArgs(std::vector<KShim::string> args) const;
    std::filesystem::path makeAbsouteCommand(const std::filesystem::path &_path) const;


    std::filesystem::path m_app;
    std::vector<KShim::string> m_args;
    std::vector<std::pair<KShim::string, KShim::string>> m_env;
    std::vector<char> m_rawData;

};

#endif // KSHIMDATA_H
