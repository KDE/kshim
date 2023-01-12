#include "test_config.h"
#include "kshimdata.h"

int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);

    const auto kshimgen =
            (KShimTest::binaryDir() / KSTRING("kshimgen"s)).replace_extension(KShimLib::exeSuffix);
    const auto target =
            (KShimTest::binaryDir() / KSTRING("path_test"s)).replace_extension(KShimLib::exeSuffix);

    TEST_EQ(KShimLib::run(KShimData(kshimgen),
                          { KSTRING("--create"s), KSTRING("test"s), KShimLib::string(target) }),
            0);

    // TODO: this depends on the result of path_test
    TEST_EQ(KShimLib::run(KShimData(target), {}), 0);
    kLog << "End";
}
