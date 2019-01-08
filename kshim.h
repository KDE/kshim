#ifndef KSHIM_H
#define KSHIM_H

#include <iostream>
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
}

class KLog
{
public:
    KLog();
    ~KLog();

    KLog &log() { return  *this; }

private:
    static bool s_doLog;

    template<typename T>
    friend KLog & operator<<(KLog &, const T&);
};

#define kLog KLog().log()

template <typename T>
KLog &operator<< (KLog &log, const T &t) {
    if (KLog::s_doLog) {
        std::cout << t;
    }
    return log;
}
#endif // KSHIM_H
