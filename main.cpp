#include "kshim.h"
#include "kshimdata.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    KShimData data;
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    if (!data.isShim()) {
        if (argc > 1) {
            const std::string &arg1 = args[1];
            if (arg1 == "--create") {
                return KShim::createShim(data, args) ? 0 : -1;
            }
        } else {
            std::cout << "Usage: --create targe command" << std::endl;
        }
        return -1;
    } else {
        return KShim::run(data, args);
    }
}
