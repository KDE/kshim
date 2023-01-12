#include "test_config.h"

int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);
    const auto binaryName = KShimLib::binaryName();
    TEST_EQ(binaryName,
            (KShimTest::binaryDir() / KSTRING("binaryName_test"s))
                    .replace_extension(KShimLib::exeSuffix));
    kLog << "End";
    return 0;
}
