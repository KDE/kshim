#include "kshim.h"

#include <assert.h>

#define TEST(X) kLog << #X ": " << (X ? "true":"false"); if (!X) kLog << "Failed"  ;assert(X);
#define TEST_EQ(X, Y) kLog << #X  "==" #Y ": " <<  X << "==" << Y; if(X != Y)kLog << "Failed"; assert(X==Y);

int main()
{
    KLog::setLoggingEnabled(true);
    const auto path = KShimLib::path(KSTRING("/foo/bar.txt"s));
    const auto foo_bar = KShimLib::path(KSTRING("/foo"s)) / KSTRING("bar.txt");
    kLog << path;
    kLog << path.parent_path();
    kLog << foo_bar;
    TEST_EQ(path.parent_path(), KShimLib::path(KSTRING("/foo"s)));
    TEST_EQ(path.filename(), KShimLib::path(KSTRING("bar.txt"s)));
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
    TEST_EQ(path.wstring(), L"\\foo\\bar.txt"s);
    TEST_EQ(path.string(), "\\foo\\bar.txt"s);
    TEST(KShimLib::path(KSTRING("C:\\"s)).is_absolute());
#else
    const auto absPath = KShimLib::path(KSTRING("/foo/bar"s));
    TEST(absPath.is_absolute());
    TEST(KShimLib::path(absPath).is_absolute());
    TEST(KShimLib::path(KSTRING("/"s)).is_absolute());
    TEST_EQ(path.string(), "/foo/bar.txt"s);
#endif
    TEST_EQ(KShimLib::path(KSTRING("../foo"s)).is_absolute(), false);
    TEST_EQ(KShimLib::path(KSTRING("foo"s)).is_absolute(), false);
    return 0;
}
