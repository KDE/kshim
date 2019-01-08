#ifndef KSHIM_H
#define KSHIM_H

#include <string>
#include <vector>

class KShimData;

namespace KShim
{
constexpr char dirSep() {
#ifdef _WIN32
    return '\\';
#else
    return  '/';
#endif
}
    int run(const KShimData &data, const std::vector<std::string> &args);
    bool createShim(KShimData &shimData, const std::vector<std::string> &args);
    std::string binaryName();
};

#endif // KSHIM_H
