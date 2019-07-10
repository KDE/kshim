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

#include "config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

class KShimData;

namespace KShimLib {
int run(const KShimData &data, const std::vector<string> &args);
KShimLib::path binaryName();
KShimLib::string getenv(const KShimLib::string &var);
}

class KLog
{
public:
    enum class Type { Debug, Error };
    KLog(Type t);
    KLog(const KLog &other);
    ~KLog();

    KLog &log();

    Type type() const;

    static bool loggingEnabled();
    static void setLoggingEnabled(bool loggingEnabled);

private:
    bool doLog() const;
    Type m_type;
    std::shared_ptr<KShimLib::stringstream> m_stream;

    static bool s_loggingEnabled;

    friend KLog &operator<<(KLog &log, const KShimLib::path &t);
    friend KLog &operator<<(KLog &log, const std::string &t);

    template<typename T>
    friend KLog &operator<<(KLog &, const T &);
};
#define kLog KLog(KLog::Type::Debug).log()
#define kLog2(X) KLog(X).log()

KLog &operator<<(KLog &log, const KShimLib::path &t);
KLog &operator<<(KLog &log, const std::string &t);

template<typename T>
KLog &operator<<(KLog &log, const T &t)
{
    if (log.doLog()) {
        *log.m_stream << t;
    }
    return log;
}

#endif // KSHIM_H
