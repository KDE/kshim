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

#include <algorithm>

void KShimGenPrivate::setPayload(const KShimLib::path &dest, const std::vector<uint8_t> &payload)
{
    std::vector<char> dataOut;
    {
        std::ifstream in(dest.string(), std::ios::in | std::ios::binary);
        if (!in.is_open()) {
            kLog2(KLog::Type::Error) << "Failed to open in: " << dest.string();
            return exit(1);
        }
        in.seekg(0, std::ios::end);
        dataOut.resize(in.tellg(), 0);
        in.seekg(0);
        in.read(dataOut.data(), dataOut.size());
        in.close();
    }

    // look for the end mark and search for the start from there
    const KShimPayLoad findPayLoad { 0, KShimDataDef };
    const char *_start = reinterpret_cast<const char *>(&findPayLoad);
    auto cmdIt = std::search(dataOut.begin(), dataOut.end(), _start, _start + sizeof(KShimPayLoad));
    if (cmdIt == dataOut.end()) {
        kLog2(KLog::Type::Error) << "Failed to patch binary, please report your compiler";
        exit(1);
    }
    KShimPayLoad *outPayLoad = reinterpret_cast<KShimPayLoad *>(&*cmdIt);

    if (payload.size() > KShimLib::DataStorageSize) {
        kLog2(KLog::Type::Error) << "Data buffer is too small " << payload.size() << " > "
                                 << KShimLib::DataStorageSize << " :\n"
                                 << payload.data();
        return exit(1);
    }
    outPayLoad->size = payload.size();
    std::copy(payload.cbegin(), payload.cend(), outPayLoad->cmd);

    {
        std::ofstream out(dest.string(), std::ios::out | std::ios::binary);
        if (!out.is_open()) {
            kLog2(KLog::Type::Error) << "Failed to open out: " << dest.string();
            exit(1);
        }
        out.write(dataOut.data(), dataOut.size());
        out.close();
    }
}
