#include "kshim.h"

#include <assert.h>

#define TEST(X) kLog << #X ": " << (X ? "true":"false"); if (!X) { kLog2(KLog::Type::Fatal) << "Failed";}
#define TEST_EQ(X, Y) kLog << #X  "==" #Y ": " <<  X << "==" << Y; if(X != Y) { kLog2(KLog::Type::Fatal)<< "Failed";}

int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);
    const auto path = KShimLib::path(KSTRING("/foo/bar.txt"s));
    const auto foo_bar = KShimLib::path(KSTRING("/foo"s)) / KSTRING("bar.txt");
    TEST_EQ(path, foo_bar);
    TEST_EQ(path.parent_path(), KShimLib::path(KSTRING("/foo"s)));
    TEST_EQ(path.filename(), KShimLib::path(KSTRING("bar.txt"s)));

    auto exe = path;
    TEST_EQ(exe.replace_extension(KShimLib::path(KSTRING(".exe"s))), KShimLib::path(KSTRING("/foo/bar.exe"s)));
    TEST_EQ(exe.extension(), KShimLib::path(KSTRING(".exe"s)));
    TEST_EQ(exe.replace_extension(KShimLib::path(KSTRING(".png"s))), KShimLib::path(KSTRING("/foo/bar.png"s)));
    TEST_EQ(KShimLib::path(KSTRING("/foo/bar"s)).replace_extension(KShimLib::path(KSTRING(".png"s))), KShimLib::path(KSTRING("/foo/bar.png"s)));
    TEST_EQ(KShimLib::path(KSTRING("/foo/bar"s)).extension(), KShimLib::path());

#ifdef _WIN32
    const auto absPath = KShimLib::path(KSTRING("C:/foo/bar.exe"s));
    TEST(absPath.is_absolute());
    TEST(KShimLib::path(absPath).is_absolute());
    TEST_EQ(absPath, KShimLib::path(KSTRING("C:\\foo\\bar.exe"s)));
    TEST_EQ(path.wstring(), L"/foo/bar.txt"s);
    TEST_EQ(KShimLib::path(path).make_preferred().wstring(), L"\\foo\\bar.txt"s);
    TEST(KShimLib::path(KSTRING("C:\\"s)).is_absolute());
#else
    const auto absPath = KShimLib::path(KSTRING("/foo/bar"s));
    TEST(absPath.is_absolute());
    TEST(KShimLib::path(absPath).is_absolute());
    TEST(KShimLib::path(KSTRING("/"s)).is_absolute());
#endif
    TEST_EQ(path.string(), "/foo/bar.txt"s);
    TEST_EQ(KShimLib::path(KSTRING("../foo"s)).is_absolute(), false);
    TEST_EQ(KShimLib::path(KSTRING("foo"s)).is_absolute(), false);
    kLog << "End";
    return 0;
}
