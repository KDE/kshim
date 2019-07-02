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

#ifndef KSHIM_H
#define KSHIM_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

class KShimData;


namespace KShim
{
#ifdef _WIN32
#define KSTRING(X) L##X
using string = std::wstring;
using stringstream = std::wstringstream;
#else
#define KSTRING(X) X
using string = std::string;
using stringstream = std::stringstream;
#endif

int run(const KShimData &data, const std::vector<string> &args);
bool createShim(KShimData &shimData, const KShim::string &appName, const std::filesystem::path &target, const std::vector<KShim::string> &args, const std::vector<KShim::string> &env);
std::filesystem::path binaryName();
KShim::string getenv(const KShim::string &var);

int main(const std::vector<string> &args);
}

class KLog
{
public:
    enum class Type
    {
        Debug,
        Error
    };
    KLog(Type t);
    KLog(const KLog &other);
    ~KLog();

    KLog &log();

    Type type() const;

private:
    bool doLog() const;
    Type m_type;
    std::shared_ptr<KShim::stringstream> m_stream;

    template<typename T>
    friend KLog & operator<<(KLog &, const T&);
};
#define kLog KLog(KLog::Type::Debug).log()
#define kLog2(X) KLog(X).log()

template <typename T>
KLog &operator<< (KLog &log, const T &t) {
    if (log.doLog()) {
        *log.m_stream << t;
    }
    return log;
}

template <>
inline KLog &operator<< (KLog &log, const std::filesystem::path &t) {
#ifdef _WIN32
    log << t.wstring();
#else
    log << t.string();
#endif
    return log;
}


#endif // KSHIM_H
