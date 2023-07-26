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

#ifdef __FreeBSD__
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <libprocstat.h>
#endif

extern char **environ;

std::filesystem::path KShimLib::binaryName()
{
    static std::filesystem::path _path = [] {
        size_t size;
#if __APPLE__
        string out(PROC_PIDPATHINFO_MAXSIZE, 0);
        size = proc_pidpath(getpid(), const_cast<char *>(out.data()), out.size());
#elif defined(__FreeBSD__)
        string out(PATH_MAX, 0);
        int error, name[4];
        size_t len = PATH_MAX;

        name[0] = CTL_KERN;
        name[1] = KERN_PROC;
        name[2] = KERN_PROC_PATHNAME;
        name[3] = getpid();

        error = sysctl(name, nitems(name), const_cast<char *>(out.data()), &len, NULL, 0);
        len--; // cut off zero-terminator
        out.resize(len);
        size = len;
#else
        string out;
        do {
            out.resize(out.size() + 1024);
            size = readlink("/proc/self/exe", const_cast<char *>(out.data()), out.size());
        } while (out.size() == size);
#endif
        if (size > 0) {
            out.resize(size);
        } else {
            kLog2(KLog::Type::Error) << "Failed to locate shimgen";
            exit(1);
        }
        return std::filesystem::path(out);
    }();
    return _path;
}

int KShimLib::run(const KShimData &data, const std::vector<KShimLib::string_view> &args)
{
    for (auto &var : data.env()) {
        kLog << "setenv: " << var.first << "=" << var.second;
        if (var.second.empty()) {
            unsetenv(var.first.data());
        } else {
            setenv(var.first.data(), var.second.data(), true);
        }
    }
    std::vector<char *> arguments;
    auto addArg = [&arguments](const KShimLib::string_view &s) {
        arguments.push_back(const_cast<char *>(s.data()));
    };
    const auto app = data.appAbsWithOverride().string();
    addArg(app);
    for (const auto &s : data.args()) {
        addArg(s);
    }
    for (const auto &s : args) {
        addArg(s);
    }
    // the args need to end with a null pointer
    arguments.push_back(nullptr);

    {
        auto log = kLog << "Command:";
        log << app;
        for (const char *s : arguments) {
            if (s) {
                log << " " << s;
            }
        }
    }
    pid_t pid;
    int status = posix_spawn(&pid, app.data(), NULL, NULL, arguments.data(), environ);
    if (status == 0) {
        if (waitpid(pid, &status, 0) != -1) {
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                if(WIFSIGNALED(status)) {
                    if(WCOREDUMP(status)) {
                        kLog2(KLog::Type::Error) << "KShim: the child process produced a core dump";
                    }
                    if(WTERMSIG(status)) {
                        kLog2(KLog::Type::Error) << "KShim: the child process was terminated";;
                    }
                }
            }
        } else {
            kLog2(KLog::Type::Error) << "KShim: waitpid error";
        }
    } else {
        kLog2(KLog::Type::Error) << "KShim: posix_spawn: " << strerror(status);
    }
    return -1;
}

KShimLib::string KShimLib::getenv(const KShimLib::string_view &var,
                                  const KShimLib::string_view &fallback)
{
    const char *env = ::getenv(var.data());
    if (env) {
        return { env };
    }
    return fallback.empty() ? "" : fallback.data();
}

std::filesystem::path KShimLib::findInPath(const std::filesystem::path &path)
{
    auto path_env = std::stringstream(KShimLib::getenv("PATH"));
    std::string dir;
    while (std::getline(path_env, dir, ':')) {
        const auto file = std::filesystem::path(dir) / path;
        if (file != KShimLib::binaryName()) {
            struct stat sb;
            if (stat(file.string().data(), &sb) == 0 && sb.st_mode & S_IXUSR) {
                kLog << "Found: " << file << " for " << path;
                return file;
            }
        }
    }
    kLog2(KLog::Type::Fatal) << "Failed to locate" << path;
    return {};
}

bool KShimLib::exists(const std::filesystem::path &path)
{
    struct stat sb;
    return stat(path.string().data(), &sb) == 0;
}
