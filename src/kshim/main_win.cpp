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

#include <windows.h>
#include <shellapi.h>

int _main()
{
    const auto commandLine = GetCommandLineW();
    size_t argc;
    wchar_t **argv = CommandLineToArgvW(commandLine, reinterpret_cast<int *>(&argc));

    std::vector<KShimLib::string> args;
    args.resize(argc);
    for (size_t i = 0; i < argc; ++i) {
        args[i] = argv[i];
    }
    return KShim::main(args);
}

int main()
{
    return _main();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, char *, int)
{
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *dummy;
        _wfreopen_s(&dummy, L"CONOUT$", L"w", stdout);
        setvbuf(stdout, nullptr, _IONBF, 0);

        _wfreopen_s(&dummy, L"CONOUT$", L"w", stderr);
        setvbuf(stderr, nullptr, _IONBF, 0);
        std::ios::sync_with_stdio();
    }
    return _main();
}
