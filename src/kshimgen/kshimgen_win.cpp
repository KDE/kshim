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

#include <windows.h>

namespace {

// https://docs.microsoft.com/en-us/previous-versions/ms997538(v=msdn.10)?redirectedfrom=MSDN
// #pragmas are used here to insure that the structure's
// packing in memory matches the packing of the EXE or DLL.
#pragma pack(push)
#pragma pack(2)
typedef struct
{
    BYTE bWidth; // Width, in pixels, of the image
    BYTE bHeight; // Height, in pixels, of the image
    BYTE bColorCount; // Number of colors in image (0 if >=8bpp)
    BYTE bReserved; // Reserved
    WORD wPlanes; // Color Planes
    WORD wBitCount; // Bits per pixel
    DWORD dwBytesInRes; // how many bytes in this resource?
    WORD nID; // the ID
} GRPICONDIRENTRY;

typedef struct
{
    WORD idReserved; // Reserved (must be 0)
    WORD idType; // Resource type (1 for icons)
    WORD idCount; // How many images?
    GRPICONDIRENTRY idEntries[1]; // The entries for each image
} GRPICONDIR;
#pragma pack(pop)

std::wstring printableRCType(const wchar_t *type)
{
    if (IS_INTRESOURCE(type)) {
        return std::to_wstring(reinterpret_cast<const int64_t>(type));
    }
    return type;
}

BOOL CALLBACK resourceCallback(HMODULE, wchar_t *, wchar_t *name, intptr_t *contextPtr)
{
    auto dest = reinterpret_cast<wchar_t **>(contextPtr);
    if (IS_INTRESOURCE(name)) {
        *dest = name;
    } else {
        size_t len = wcslen(name) + 1;
        wchar_t *buffer = new wchar_t[len];
        wcscpy_s(buffer, len, name);
        *dest = buffer;
    }
    return false;
}

bool copyResource(HMODULE exe, HANDLE updateHandle, const wchar_t *id, const wchar_t *type)
{
    auto iconPos = FindResourceW(exe, id, type);
    if (!iconPos) {
        kLog2(KLog::Type::Error) << "Failed to find resource: " << printableRCType(type) << ": "
                                 << printableRCType(id);
        return false;
    }
    auto icon = LoadResource(exe, iconPos);
    if (!icon) {
        kLog2(KLog::Type::Error) << "Failed to load resource: " << printableRCType(type) << ": "
                                 << printableRCType(id);
        return false;
    }
    auto lock = LockResource(icon);
    if (!lock) {

        kLog2(KLog::Type::Error) << "Failed to lock resource: " << printableRCType(type) << ": "
                                 << printableRCType(id);
        return false;
    }
    if (!UpdateResourceW(updateHandle, type, id, 1033, lock, SizeofResource(exe, iconPos))) {
        kLog2(KLog::Type::Error) << "Failed to upload resource: " << printableRCType(type) << ": "
                                 << printableRCType(id);
        return false;
    }
    return true;
};
}
namespace KShimGenPrivate {

void updateIcon(const KShimLib::path &src, const KShimLib::path &dest)
{
    auto exe = LoadLibraryExW(src.wstring().data(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!exe) {
        kLog << "Failed to load exe for icon: " << src;
        return;
    }
    wchar_t *iconGroupName = nullptr;
    EnumResourceNamesW(exe, RT_GROUP_ICON, (ENUMRESNAMEPROCW)resourceCallback,
                       reinterpret_cast<intptr_t>(&iconGroupName));
    if (iconGroupName) {
        auto infoPos = FindResourceW(exe, iconGroupName, RT_GROUP_ICON);
        if (!infoPos) {
            return;
        }
        auto infoRc = LoadResource(exe, infoPos);
        if (!infoRc) {
            kLog2(KLog::Type::Error)
                    << "Failed to find icon info: " << printableRCType(iconGroupName);
            return;
        }
        auto lock = LockResource(infoRc);
        if (!lock) {
            kLog2(KLog::Type::Error)
                    << "Failed to lock icon info: " << printableRCType(iconGroupName);
            return;
        }
        kLog << "Found icon info: " << printableRCType(iconGroupName);
        auto info = reinterpret_cast<GRPICONDIR *>(lock);
        if (!info) {
            kLog2(KLog::Type::Error) << "Failed to load icon info";
        } else {
            auto updateHandle = BeginUpdateResourceW(dest.wstring().data(), false);
            if (!updateHandle) {
                kLog2(KLog::Type::Error) << "Failed to BeginUpdateResource: " << src;
                exit(1);
            }
            copyResource(exe, updateHandle, iconGroupName, RT_GROUP_ICON);
            for (int64_t i = 0; i < info->idCount; ++i) {
                copyResource(exe, updateHandle, MAKEINTRESOURCEW(info->idEntries[i].nID), RT_ICON);
            }
            kLog << "Copied: " << info->idCount << " icons";

            if (!IS_INTRESOURCE(iconGroupName)) {
                delete[] iconGroupName;
            }
            if (!EndUpdateResourceW(updateHandle, false)) {
                kLog2(KLog::Type::Error) << "Failed to EndUpdateResource: " << src;
                exit(1);
            }
        }
    }
    if (!FreeLibrary(exe)) {
        kLog2(KLog::Type::Error) << "Failed to FreeLibrary: " << src;
        exit(1);
    }
}

}
