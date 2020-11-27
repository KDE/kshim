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

#include <algorithm>
#include <comdef.h>
#include <windows.h>

using namespace std;

KShimLib::path KShimLib::binaryName()
{
    static const KShimLib::path path = [] {
        std::wstring buf;
        size_t size;
        do {
            buf.resize(buf.size() + 1024);
            size = GetModuleFileNameW(nullptr, const_cast<wchar_t *>(buf.data()),
                                      static_cast<DWORD>(buf.size()));
        } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
        buf.resize(size);
        return KShimLib::path(buf);
    }();
    return path;
}

bool KShimLib::exists(const KShimLib::path &path)
{
    DWORD dwAttrib = GetFileAttributesW(path.wstring().data());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

KShimLib::path KShimData::findInPath(const KShimLib::path &path) const
{
    auto find = [](const std::wstring &dir, const KShimLib::path &name) {
        const std::wstring ext = name.extension().empty() ? L".exe" : name.extension();
        std::wstring buf;
        size_t size;
        wchar_t *filePart;
        size = SearchPathW(dir.data(), name.wstring().data(), ext.data(), 0, nullptr, &filePart);
        if (size == 0) {
            return std::wstring();
        }
        buf.resize(size);
        size = SearchPathW(dir.data(), name.wstring().data(), ext.data(),
                           static_cast<DWORD>(buf.size()), buf.data(), &filePart);
        if (size > buf.size()) {
            kLog2(KLog::Type::Fatal) << "SearchPathW failed to find " << name
                                     << " error: " << _com_error(GetLastError()).ErrorMessage()
                                     << buf.size() << " " << size;
        }
        buf.resize(size);
        return buf;
    };
    auto path_env = std::wstringstream(KShimLib::getenv(L"PATH"));
    std::wstring dir;
    while (std::getline(path_env, dir, L';')) {
        auto tmp = find(dir, path.wstring());
        if (!tmp.empty() && tmp != KShimLib::binaryName()) {
            kLog << "Found: " << tmp << " for " << path;
            return tmp;
        }
    }
    kLog2(KLog::Type::Fatal) << "Failed to locate" << path;
    return {};
}

int KShimLib::run(const KShimData &data, const std::vector<KShimLib::string_view> &args)
{
    for (auto var : data.env()) {
        kLog << "SetEnvironmentVariable: " << var.first << "=" << var.second;
        SetEnvironmentVariableW(var.first.data(), var.second.empty() ? nullptr : var.second.data());
    }
    // TODO: pass environment
    STARTUPINFOW info = {};
    info.cb = sizeof(info);
    PROCESS_INFORMATION pInfo = {};
    const auto arguments = data.formatCommand(args);
    kLog << data.appAbs() << " " << arguments;
    kLog << "CommandLength: " << arguments.size();

    if (!CreateProcessW(data.appAbs().wstring().c_str(), const_cast<wchar_t *>(arguments.c_str()),
                        nullptr, nullptr, true,
                        INHERIT_PARENT_AFFINITY | CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr,
                        &info, &pInfo)) {
        const auto error = GetLastError();
        kLog2(KLog::Type::Error) << "Failed to start target" << _com_error(error).ErrorMessage();
        return static_cast<int>(error);
    }
    WaitForSingleObject(pInfo.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pInfo.hProcess, &exitCode);

    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
    return static_cast<int>(exitCode);
}

KShimLib::string KShimLib::getenv(const KShimLib::string_view &var,
                                  const KShimLib::string_view &fallback)
{
    const auto size = GetEnvironmentVariableW(var.data(), nullptr, 0);
    if (!size) {
        return fallback.empty() ? L""s : fallback.data();
    }
    KShimLib::string out(size, 0);
    GetEnvironmentVariableW(var.data(), const_cast<wchar_t *>(out.data()), size);
    out.resize(size - 1);
    return out;
}
