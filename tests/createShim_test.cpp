#include "test_config.h"
#include "kshimdata.h"

int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);

    const auto kshimgen =
            (KShimTest::binaryDir() / KSTRING("kshimgen"s)).replace_extension(KShimLib::exeSuffix);

    TEST_EQ(KShimLib::run(KShimData(kshimgen),
                          { KSTRING("--create"s), KSTRING("test"s), KShimLib::string(kshimgen) }),
            0);

    TEST_EQ(KShimLib::run((std::filesystem::current_path() / "test"s)
                                  .replace_extension(KShimLib::exeSuffix),
                          { KSTRING("-h"s) }),
            0);
    kLog << "End";
}
