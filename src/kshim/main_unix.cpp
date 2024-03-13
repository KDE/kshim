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
#include "kshimmain.h"
#include "kshimdata.h"

#include <vector>

int main(int argc, char *argv[])
{
    std::vector<KShimLib::string_view> args;
    args.resize(argc);
    for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
        args[i] = argv[i];
    }

    std::vector<uint8_t> payload;
    {
        std::ifstream in(KShimLib::binaryName().string(), std::ios::in | std::ios::binary);
        if (!in.is_open()) {
            kLog2(KLog::Type::Error) << "Failed to open in: " << KShimLib::binaryName().string();
            return 1;
        }

        int64_t size;
        in.seekg(-signed(sizeof(size)), std::ios::end);
        in.read(reinterpret_cast<char *>(&size), sizeof(size));
        in.seekg(-signed(sizeof(size)) - size, std::ios::end);
        payload.resize(size);
        in.read(reinterpret_cast<char *>(payload.data()), size);
        in.close();
    }

    return KShim::main(payload, args);
}
