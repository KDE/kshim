/*
    Copyright Hannah von Reth <vonreth@kde.org>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include "kshim.h"
#include "kshimdata.h"

#include <unistd.h>
#include <sys/stat.h>
#include <spawn.h>
#include <sys/wait.h>
#include <cstring>

#ifdef __APPLE__
#include <libproc.h>
#endif

using namespace std;

extern char **environ;

bool KShim::isAbs(const std::string &s)
{
    return s.length() >= 1 && s[0] == KShim::dirSep();
}

string KShim::binaryName()
{
    size_t size;
#if __APPLE__
    string out(PROC_PIDPATHINFO_MAXSIZE, 0);
    size = proc_pidpath(getpid(), const_cast<char*>(out.data()), out.size());
#else
    string out;
    do {
        out.resize(out.size() + 1024);
        size = readlink("/proc/self/exe", const_cast<char*>(out.data()), out.size());
    } while (out.size() == size);
#endif
    if (size>0) {
        out.resize(size);
    } else {
        cerr << "Failed to locate shimgen" << endl;
        exit(1);
    }
    return out;
}

int KShim::run(const KShimData &data, int argc, char *argv[])
{
    for (auto var : data.env())
    {
        kLog << "setenv: " << var.first << "=" << var.second;
        setenv(var.first.c_str(), var.second.c_str(), true);
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
