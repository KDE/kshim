#include "kshim.h"

#include <assert.h>

int main()
{
    KLog::setLoggingEnabled(true);
    const auto path = KShimLib::path(KSTRING_LITERAL("/foo/bar.txt"));
    const auto foo_bar = KShimLib::path(KSTRING_LITERAL("/foo")) / KSTRING_LITERAL("bar.txt");
    kLog << path;
    kLog << path.parent_path();
    kLog << foo_bar;
    assert(path.parent_path() == KShimLib::path(KSTRING_LITERAL("/foo")));
    assert(path == foo_bar);

    auto exe = path;
    kLog << exe.replace_extension(KSTRING_LITERAL(".exe"));
    assert(exe == KShimLib::path(KSTRING_LITERAL("/foo/bar.exe")));
#ifdef _WIN32
    assert(KShimLib::path(KSTRING_LITERAL("C:/foo/bar.exe"))
           == KShimLib::path(KSTRING_LITERAL("C:\\foo\\bar.exe")));
    kLog << "wstring " << path.wstring();
#endif
    kLog << "string " << path.string();
    return 0;
}
