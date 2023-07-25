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

#include "kshim.h"
#include "kshimdata.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

bool KLog::s_loggingEnabled = !KShimLib::getenv(KSTRING("KSHIM_LOG")).empty();
bool KLog::s_stdLoggingEnabled = !KShimLib::getenv(KSTRING("KSHIM_LOG_STD")).empty();

KLog::KLog(KLog::Type t) : m_type(t), m_stream(new KShimLib::stringstream) {}

KLog::KLog(const KLog &other) : m_type(other.m_type), m_stream(other.m_stream) {}

KLog::~KLog()
{
    if (m_stream.use_count() == 1) {
        *this << "\n";
        const auto line = m_stream->str();
        if (s_stdLoggingEnabled) {
#ifdef _WIN32
            std::wcerr << line;
#else
            std::cerr << line;
#endif
        } else {
            switch (m_type) {
            case KLog::Type::Error:
            case KLog::Type::Fatal:
#ifdef _WIN32
                std::wcerr << line << std::endl;;
#else
                std::cerr << line << std::endl;
#endif
            case KLog::Type::Debug: {
                if (doLog()) {
                    static auto _log = [] {
                        auto home = KShimLib::getenv(KSTRING("HOME"));
                        if (home.empty()) {
                            home = KShimLib::getenv(KSTRING("USERPROFILE"));
                        }
                        const auto logPath = std::filesystem::path(home) / KSTRING(".kshim.log");
#ifdef _WIN32
#if !defined(__MINGW32__)
                        const auto &_name = logPath;
#else
                        const auto _name = logPath.string();
#endif
                        auto out = std::wofstream(_name, std::ios::app);
#else
                        auto out = std::ofstream(logPath, std::ios::app);
#endif
                        if (!out.is_open()) {
                            std::cerr << "KShim: Failed to open log \"" << logPath.string() << "\" "
                                      << strerror(errno) << std::endl;
                        }
                        out << "----------------------------\n";
                        return out;
                    }();
#ifdef _WIN32
                    OutputDebugStringW(line.data());
#endif
                    _log << line;
                    _log.flush();
                }
            }
            }
        }
    }
    if (m_type == Type::Fatal) {
        exit(-1);
    }
}

KLog &KLog::log()
{
    *this << "KShimgen " << KShimLib::version << ": ";
    return *this;
}

KLog::Type KLog::type() const
{
    return m_type;
}

bool KLog::doLog() const
{
    return loggingEnabled() || m_type != KLog::Type::Debug;
}

bool KLog::getStdLoggingEnabled()
{
    return s_stdLoggingEnabled;
}

void KLog::setStdLoggingEnabled(bool value)
{
    s_stdLoggingEnabled = value;
}

bool KLog::loggingEnabled()
{
    return s_loggingEnabled;
}

void KLog::setLoggingEnabled(bool loggingEnabled)
{
    s_loggingEnabled = loggingEnabled;
}

KLog &operator<<(KLog &log, const std::filesystem::path &t)
{
#ifdef _WIN32
    log << t.wstring();
#else
    log << t.string();
#endif
    return log;
}

KLog &operator<<(KLog &log, const std::string &t)
{
    log << t.data();
    return log;
}
