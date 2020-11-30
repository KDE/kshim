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

#ifndef KSHIMPATH_H
#define KSHIMPATH_H

#include "kshimstring.h"

#include <vector>

class KShimPath
{
public:
    KShimPath();
    KShimPath(const KShimLib::string_view &path);
    KShimPath(const KShimPath &path);
    explicit KShimPath(const KShimLib::string &path);

    bool is_absolute() const;

    KShimPath &replace_extension(const KShimPath &path);
    KShimPath parent_path() const;
    KShimPath filename() const;
    KShimPath extension() const;

    bool empty() const;

#ifdef _WIN32
    std::wstring wstring() const;
#endif
    std::string string() const;
    operator KShimLib::string() const;
    KShimPath &make_preferred();

private:
    KShimLib::char_t m_seperator;
    std::vector<KShimLib::string> m_parts;
    bool m_is_abs = false;

    friend KShimPath operator/(const KShimPath &lhs, const KShimPath &rhs);
    friend KShimPath operator/(const KShimPath &lhs, const KShimLib::string_view &rhs);
    friend bool operator==(const KShimPath &lhs, const KShimPath &rhs);
    friend bool operator!=(const KShimPath &lhs, const KShimPath &rhs);
};

KShimPath operator/(const KShimPath &lhs, const KShimPath &rhs);
KShimPath operator/(const KShimPath &lhs, const KShimLib::string_view &rhs);
bool operator==(const KShimPath &lhs, const KShimPath &rhs);
inline bool operator!=(const KShimPath &lhs, const KShimPath &rhs) {
    return !(lhs == rhs);
}

#endif // KSHIMPATH_H
