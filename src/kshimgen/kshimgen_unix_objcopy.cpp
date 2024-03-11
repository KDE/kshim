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
#include "kshimgen_p.h"

#include <unistd.h>
#include <format>

class TmpDeleterRaii
{
public:
    TmpDeleterRaii(std::filesystem::path &&path) : m_path(path) { }

    ~TmpDeleterRaii()
    {
        if (exists(m_path)) {
            remove(m_path);
        }
    }
    const auto &path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

void KShimGenPrivate::setPayload(const std::filesystem::path &dest,
                                 const std::vector<uint8_t> &payload)
{
    TmpDeleterRaii jsonFile =
            std::filesystem::temp_directory_path() / std::string("kshim-payload.json");

    std::ofstream out(jsonFile.path().string(), std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        kLog2(KLog::Type::Error) << "Failed to open out: " << jsonFile.path();
        exit(1);
    }
    out.write(reinterpret_cast<const char *>(payload.data()), static_cast<int>(payload.size()));
    out.write("\0", 1);
    out.close();

    const auto objcopy =
            KShimLib::findInPath(std::filesystem::path(KShimLib::string("llvm-objcopy")));
#if 0
    int result = KShimLib::run(KShimData(objcopy), { "--input", "binary", "--output", "elf64-x86-64", "--binary-architecture", "i386:x86-64",
                "--rename-section", ".data=.rodata,CONTENTS,ALLOC,LOAD,READONLY,DATA", jsonFile.path().string(), oFile.path().string() });
    if (result != 0) {
        kLog2(KLog::Type::Error) << "Failed to generate object: " << oFile.path();
        exit(result);
    }
#endif

    auto result =
            KShimLib::run(KShimData(objcopy),
                          { "--update-section",
                            std::string("__KSHIMDATA,__kshimdata=") + jsonFile.path().string(),
                            dest.string(), dest.string() + "2" });
    if (result != 0) {
        kLog2(KLog::Type::Error) << "Failed to generate object: " << dest;
        exit(result);
    }
}
