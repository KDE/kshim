#include "kshim.h"
#include "kshimdata.h"

#include <windows.h>

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
