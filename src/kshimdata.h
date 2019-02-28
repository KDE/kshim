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

#include <string>
#include <vector>

class KShimData
{
public:
    KShimData();

    std::string app() const;
    std::string appAbs() const;
    void setApp(const std::string &app);

    const std::vector<std::string> &args() const;
    void setArgs(const std::vector<std::string> &args);
    void addArg(const std::string &arg);

    std::vector<std::pair<std::string, std::string>> env() const;
    void setEnv(const std::vector<std::pair<std::string, std::string> > &env);
    void addEnvVar(const std::pair<std::string, std::string> &var);

    std::string formatCommand(const std::vector<std::string> &args) const;
    std::string formatArgs(const std::vector<std::string> &args) const;

    bool isShim() const;
    const std::vector<char> &rawData() const;

    std::string toJson() const;

private:
    std::string quote(const std::string &arg) const;
    std::string quoteArgs(std::vector<std::string> args) const;
    std::string makeAbsouteCommand(const std::string &_path) const;


    std::string m_app;
    std::vector<std::string> m_args;
    std::vector<std::pair<std::string, std::string>> m_env;
    std::vector<char> m_rawData;

};

#endif // KSHIMDATA_H
