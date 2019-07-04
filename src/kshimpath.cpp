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

#include "kshimpath.h"

#include <algorithm>

#ifdef _WIN32
#include <locale>
#include <codecvt>
#endif

KShimPath::KShimPath()
{

}

KShimPath::KShimPath(const KShim::string &path)
    : m_path(path)
{
#ifdef _WIN32
     std::replace(m_path.begin(), m_path.end(), '\\', '/');
#endif
}

KShimPath::KShimPath(const KShimPath &path)
    : m_path(path.m_path)
{

}

bool KShimPath::is_absolute() const
{
#ifdef _WIN32
    return m_path.size() > 1 && m_path.at(1) == ':';
#else
    return m_path.at(0) == '/';
#endif
}

KShimPath &KShimPath::replace_extension(const KShimPath &path)
{
    for (auto it = m_path.rbegin(); it != m_path.rend(); ++it)
    {
        const auto c = *it;
        if (c == '/')
        {
            break;
        }
        else if (c == '.')
        {
            const size_t pos = &*it - m_path.data();
            m_path.erase(pos, m_path.size());
            break;
        }
    }
    m_path.append(path.m_path);
    return *this;
}

KShimPath KShimPath::parent_path() const
{
    for (auto it = m_path.rbegin(); it != m_path.rend(); ++it)
    {
        const auto c = *it;
        if (c == '/')
        {
            const size_t pos = &*it - m_path.data();
            auto out = *this;
            out.m_path.erase(pos);
            return out;
        }
    }
    return {};
}

#ifdef _WIN32
std::wstring KShimPath::wstring() const
{
    return *this;
}
#endif

std::string KShimPath::string() const
{
#ifdef _WIN32
    const auto _w = wstring();
    size_t size;
    wcstombs_s(&size, nullptr, 0, _w.data(), _w.size());
    std::string out(size, 0);
    wcstombs_s(&size, const_cast<char*>(out.data()), out.size(), _w.data(), _w.size());
    out.resize(size - 1);
    return out;
#else
   return *this;
#endif
}

KShimPath::operator KShim::string() const
{
#ifdef _WIN32
    auto out = m_path;
    std::replace(out.begin(), out.end(), '/', '\\');
    return out;
#else
    return m_path;
#endif
}

KShimPath operator /(const KShimPath &lhs, const KShimPath &rhs)
{
    auto out = lhs;
    out.m_path += KSTRING_LITERAL("/") + rhs.m_path;
    return out;
}

bool operator ==(const KShimPath &lhs, const KShimPath &rhs)
{
    return lhs.m_path == rhs.m_path;
}
