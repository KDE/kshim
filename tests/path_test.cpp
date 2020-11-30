#include "kshim.h"

#include <assert.h>

#define TEST(X) kLog << X ; assert(X);
#define TEST_EQ(X, Y) kLog << X << "==" << Y; assert(X==Y);

int main()
{
    KLog::setLoggingEnabled(true);
    const auto path = KShimLib::path(KSTRING("/foo/bar.txt"s));
    const auto foo_bar = KShimLib::path(KSTRING("/foo"s)) / KSTRING("bar.txt");
    kLog << path;
    kLog << path.parent_path();
    kLog << foo_bar;
    TEST_EQ(path.parent_path(), KShimLib::path(KSTRING("/foo"s)));
    TEST_EQ(path, foo_bar);

    auto exe = path;
    kLog << exe.replace_extension(KShimLib::path(KSTRING(".exe"s)));
    TEST_EQ(exe, KShimLib::path(KSTRING("/foo/bar.exe"s)));
    TEST_EQ(exe.extension(), KShimLib::path(KSTRING(".exe"s)));

#ifdef _WIN32
    const auto absPath = KShimLib::path(KSTRING("C:/foo/bar.exe"s));
    TEST(absPath.is_absolute());
    TEST(KShimLib::path(absPath).is_absolute());
    TEST_EQ(absPath, KShimLib::path(KSTRING("C:\\foo\\bar.exe"s)));
    kLog << "wstring " << path.wstring();
#else
    const auto absPath = KShimLib::path(KSTRING("/foo/bar"s));
    TEST(absPath.is_absolute());
    TEST(KShimLib::path(absPath).is_absolute());
#endif
    kLog << "string " << path.string();
    return 0;
}
