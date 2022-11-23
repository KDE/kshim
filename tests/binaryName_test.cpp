#include "kshim.h"

#include <assert.h>

#define TEST(X) kLog << #X ": " << (X ? "true":"false"); if (!X) { kLog2(KLog::Type::Fatal) << "Failed";}
#define TEST_EQ(X, Y) kLog << #X  "==" #Y ": " <<  X << "==" << Y; if(X != Y) { kLog2(KLog::Type::Fatal)<< "Failed";}

int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);
    const auto binaryName = KShimLib::binaryName();
    TEST_EQ(binaryName,
            KShimLib::path(std::string(BINARY_DIR "/binaryName_test")
                           + std::string(KShimLib::exeSuffix)));
    kLog << "End";
    return 0;
}
