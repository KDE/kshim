#include "kshim.h"

#include <assert.h>

int main()
{
    KLog::setLoggingEnabled(true);
    const auto path = KShimLib::path(KSTRING("/foo/bar.txt"s));
    const auto foo_bar = KShimLib::path(KSTRING("/foo"s)) / KSTRING("bar.txt");
    kLog << path;
    kLog << path.parent_path();
    kLog << foo_bar;
    assert(path.parent_path() == KShimLib::path(KSTRING("/foo"s)));
    assert(path == foo_bar);

    auto exe = path;
    kLog << exe.replace_extension(KShimLib::path(KSTRING(".exe"s)));
    assert(exe == KShimLib::path(KSTRING("/foo/bar.exe"s)));
#ifdef _WIN32
    assert(KShimLib::path(KSTRING("C:/foo/bar.exe"s))
           == KShimLib::path(KSTRING("C:\\foo\\bar.exe"s)));
    kLog << "wstring " << path.wstring();
#endif
    kLog << "string " << path.string();
    return 0;
}
