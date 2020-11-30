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
#include <sstream>

#ifdef _WIN32
#include <locale>
#include <codecvt>
#endif

namespace  {
constexpr auto UNIX_SEPERATOR(KSTRING('/'));
#ifdef _WIN32
constexpr auto NATIVE_SEPERATOR(KSTRING('\\'));
#else
constexpr char NATIVE_SEPERATOR(UNIX_SEPERATOR);
#endif
}

KShimPath::KShimPath() {}

KShimPath::KShimPath(const KShimLib::string &_path)
{
    auto path = _path;
#ifdef _WIN32
    std::replace(path.begin(), path.end(), NATIVE_SEPERATOR, UNIX_SEPERATOR);
#endif
    KShimLib::stringstream ps(path);
    KShimLib::string part;
    while(std::getline(ps, part, UNIX_SEPERATOR)) {
        m_parts.push_back(part);
    }
    if (!empty()) {
#ifdef _WIN32
        m_is_abs = path.size() > 1 && path.at(1) == ':';
#else
        m_is_abs = path.at(0) == UNIX_SEPERATOR;
#endif
    }
}
KShimPath::KShimPath(const KShimLib::string_view &path) : KShimPath(KShimLib::string(path.data()))
{
}

KShimPath::KShimPath(const KShimPath &path)
    : m_parts(path.m_parts)
    , m_is_abs(path.m_is_abs)
{}

bool KShimPath::is_absolute() const
{
    return m_is_abs;
}

KShimPath &KShimPath::replace_extension(const KShimPath &path)
{
    KShimLib::string name = filename();

    auto pos = name.rfind('.');
    if (pos == KShimLib::string::npos)
    {
        name += path;
    } else {
        name = name.substr(0, pos) + KShimLib::string(path);
    }
    m_parts.pop_back();
    m_parts.push_back(name);
    return *this;
}

KShimPath KShimPath::parent_path() const
{
    KShimPath out = *this;
    if (!out.m_parts.empty()) {
        out.m_parts.pop_back();
    }
    return out;
}

KShimPath KShimPath::filename() const
{
    return m_parts.empty() ? KShimPath() : KShimPath(m_parts.back());
}

KShimPath KShimPath::extension() const
{
    KShimLib::string name = filename();

    auto pos = name.rfind('.');
    if (pos != KShimLib::string::npos)
    {
        return KShimPath(name.substr(pos));
    }
    return {};
}

bool KShimPath::empty() const
{
    return m_parts.empty();
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
    wcstombs_s(&size, const_cast<char *>(out.data()), out.size(), _w.data(), _w.size());
    out.resize(size - 1);
    return out;
#else
    return *this;
#endif
}

KShimPath::operator KShimLib::string() const
{
    if (empty()) {
        return {};
    }
    KShimLib::stringstream tmp;
    auto it = m_parts.cbegin();
    tmp << *it++;
    for(; it != m_parts.cend(); ++it) {
        tmp << NATIVE_SEPERATOR << *it;
    }
    return tmp.str();
}

KShimPath operator/(const KShimPath &lhs, const KShimPath &rhs)
{
    auto out = lhs;
    for(const auto &p : rhs.m_parts) {
        out.m_parts.push_back(p);
    }
    return out;
}

KShimPath operator/(const KShimPath &lhs, const KShimLib::string_view &rhs)
{
    return lhs / KShimPath(rhs);
}

bool operator==(const KShimPath &lhs, const KShimPath &rhs)
{
    return lhs.m_parts == rhs.m_parts;
}
