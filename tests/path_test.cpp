#include "kshim.h"

#include <assert.h>

int main()
{
    KLog::setLoggingEnabled(true);
    const auto path = KShim::path(KSTRING_LITERAL("/foo/bar.txt"));
    const auto foo_bar = KShim::path(KSTRING_LITERAL("/foo")) / KSTRING_LITERAL("bar.txt");
    kLog << path;
    kLog << path.parent_path();
    kLog << foo_bar;
    assert(path.parent_path() == KShim::path(KSTRING_LITERAL("/foo")));
    assert(path == foo_bar);

    auto exe = path;
    kLog << exe.replace_extension(KSTRING_LITERAL(".exe"));
    assert(exe == KShim::path(KSTRING_LITERAL("/foo/bar.exe")));
#ifdef _WIN32
    assert(KShim::path(KSTRING_LITERAL("C:/foo/bar.exe"))
           == KShim::path(KSTRING_LITERAL("C:\\foo\\bar.exe")));
    kLog << "wstring " << path.wstring();
#endif
    kLog << "string " << path.string();
    return 0;
}
