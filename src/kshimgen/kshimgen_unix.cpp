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

void KShimGenPrivate::patchBinary(std::vector<char> &dataOut, const std::vector<uint8_t> &payload)
{
    if (payload.size() > KShimLib::DataStorageSize) {
        kLog2(KLog::Type::Error) << "Data buffer is too small " << payload.size() << " > "
                                 << KShimLib::DataStorageSize << " :\n"
                                 << payload.data();
        exit(1);
    }
    // look for the end mark and search for the start from there
    const KShimPayLoad findPayLoad { 0, KShimDataDef };
    auto search = [&](auto start) {
        return std::search(start, dataOut.end(), reinterpret_cast<const char *>(&findPayLoad),
                           reinterpret_cast<const char *>(&findPayLoad) + sizeof(KShimPayLoad));
    };
    int count = 0;
    for (auto cmdIt = search(dataOut.begin()); cmdIt != dataOut.end();
         count++, cmdIt = search(cmdIt)) {
        KShimPayLoad *outPayLoad = reinterpret_cast<KShimPayLoad *>(&*cmdIt);
        outPayLoad->size = payload.size();
        std::copy(payload.cbegin(), payload.cend(), outPayLoad->cmd);
    }
    // mac universal binaries might contain more than one compilation
    // replace all occurences of the placeholder KSHIM_MULTI_ARCH_COUNT
    if (count != KSHIM_MULTI_ARCH_COUNT) {
        kLog2(KLog::Type::Error) << "Expected " << KSHIM_MULTI_ARCH_COUNT << " occurances, found"
                                 << count;
        kLog2(KLog::Type::Error) << "Failed to patch binary, please report your compiler.";
        exit(1);
    }
}
