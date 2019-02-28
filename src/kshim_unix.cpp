#include "kshim.h"
#include "kshimdata.h"

#include <unistd.h>
#include <sys/stat.h>
#include <spawn.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

extern char **environ;

int KShim::run(const KShimData &data, int argc, char *argv[])
{
    for (auto var : data.env())
    {
        kLog << "putenv: " << var;
        putenv(const_cast<char*>(var.c_str()));
    }

    int pos = 0;
    const size_t size = static_cast<size_t>(argc) + data.args().size() + 1;
    char **args = new char*[size];
    auto addArg = [&pos, &args](const string &s)
    {
        args[pos] = new char[s.size()+1];
        strcpy(args[pos++], s.c_str());
    };
    addArg(data.appAbs());
    for (const auto &s : data.args()) {
        addArg(s);
    }
    for (int i = 1; i < argc; ++i) {
        args[pos++] = argv[i];
    }
    args[pos] = nullptr;
    {
        auto log = kLog << "Command:";
        log << data.appAbs();
        for (size_t i = 0; i < size - 1; ++i){
            log << " " << args[i];
        }
    }
    pid_t pid;
    int status;
    status = posix_spawn(&pid, data.appAbs().c_str(), NULL, NULL, args, environ);
    if (status == 0) {
        if (waitpid(pid, &status, 0) != -1) {
            return status;
        } else {
            cerr << "KShim: waitpid error" << endl;
        }
    } else {
        cerr << "KShim: posix_spawn: " << strerror(status) << endl;
    }
    return -1;
}
