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
#include <shellapi.h>

using namespace std;

std::filesystem::path KShim::binaryName()
{
    static const std::filesystem::path path = [] {
            std::wstring buf;
            size_t size;
            do {
                buf.resize(buf.size() + 1024);
                size = GetModuleFileNameW(nullptr, buf.data(),
                                          static_cast<DWORD>(buf.size()));
            } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
            buf.resize(size);
            return buf;
        }();
    return path;
}

int KShim::run(const KShimData &data, const std::vector<KShim::string> &args)
{
    for (auto var : data.env())
    {
        kLog << "SetEnvironmentVariable: " << var.first << "=" << var.second;
        SetEnvironmentVariableW(var.first.data(), var.second.data());
    }
    // TODO: pass environment
    STARTUPINFOW info = {};
    info.cb = sizeof (info);
    PROCESS_INFORMATION pInfo = {};
    const auto arguments = data.formatCommand(args);
    kLog << data.appAbs() << " " << arguments;
    kLog << "CommandLength: " << arguments.size();

    if(!CreateProcessW(data.appAbs().wstring().c_str(), const_cast<wchar_t*>(arguments.c_str()), nullptr, nullptr, true, INHERIT_PARENT_AFFINITY | CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &info, &pInfo))
    {
        const auto error = GetLastError();
        kLog2(KLog::Type::Error) << "Failed to start target" << error;
        return error;
    }
    WaitForSingleObject(pInfo.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pInfo.hProcess, &exitCode);

    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
    return static_cast<int>(exitCode);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, wchar_t *, int)
{
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *dummy;
        _wfreopen_s(&dummy, L"CONOUT$", L"w", stdout);
        setvbuf(stdout, nullptr, _IONBF, 0);

        _wfreopen_s(&dummy, L"CONOUT$", L"w", stderr);
        setvbuf(stderr, nullptr, _IONBF, 0);
        std::ios::sync_with_stdio();
    }
    const auto commandLine = GetCommandLineW();
    int argc;
    wchar_t **argv = CommandLineToArgvW(commandLine, &argc);

    std::vector<KShim::string> args;
    args.resize(argc);
    for(size_t i=0; i < argc; ++i)
    {
        args[i] = argv[i];
    }

    return KShim::main(args);
}


KShim::string KShim::getenv(const KShim::string &var)
{
    const auto size = GetEnvironmentVariableW(var.data(), nullptr, 0);
    KShim::string out(size, 0);
    GetEnvironmentVariableW(var.data(), out.data(), size);
    out.resize(wcslen(out.data()));
    return out;
}
