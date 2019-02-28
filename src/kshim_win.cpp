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

#include <windows.h>

using namespace std;

bool KShim::isAbs(const string &s)
{
    return s.length() >= 2 && s[1] == ':';
}

int KShim::run(const KShimData &data, int argc, char *argv[])
{
    for (auto var : data.env())
    {
        kLog << "SetEnvironmentVariable: " << var.first << "=" << var.second;
        SetEnvironmentVariable(var.first.c_str(), var.second.c_str());
    }
    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }
    // TODO: pass environment
    STARTUPINFO info = {};
    info.cb = sizeof (info);
    PROCESS_INFORMATION pInfo = {};
    const auto arguments = data.formatCommand(args);
    kLog << data.appAbs() << " " << arguments;
    kLog << "CommandLength: " << arguments.size();
    if(!CreateProcessA(const_cast<char*>(data.appAbs().c_str()), const_cast<char*>(arguments.c_str()), nullptr, nullptr, true, INHERIT_PARENT_AFFINITY, nullptr, nullptr, &info, &pInfo))
    {
        return -1;
    }
    WaitForSingleObject(pInfo.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pInfo.hProcess, &exitCode);

    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
    return static_cast<int>(exitCode);
}
