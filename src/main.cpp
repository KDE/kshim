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

#include <iostream>
#include <string>
#include <vector>

#include "args.hxx"

int main(int argc, char *argv[])
{
    KShimData data;
    if (!data.isShim()) {
        args::ArgumentParser parser("KShimGen.", "A simple application wrapper.");
        args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
        args::NargsValueFlag<std::string> create(parser, "shim target", "Create a shim", {"create"}, 2);
        args::ValueFlagList<std::string> env(parser, "env", "Sets an environment variable before calling the target", {'e', "env"});
        args::PositionalList<std::string> create_args(parser, "args", "Arguments passed to the target");
        args::CompletionFlag completion(parser, {"create", "env"});

        try {
            parser.ParseCLI(argc, argv);

        } catch (args::Completion e) {
            std::cout << e.what();
            return 0;
        } catch (args::Help) {
            std::cout << parser;
            return 0;
        } catch (args::ParseError e) {
            std::cerr << e.what() << std::endl;
            std::cerr << parser;
            return 1;
        }
        if(create) {
            const std::vector<std::string> shin_args = create.Get();
            return KShim::createShim(data, shin_args[0], shin_args[1], create_args.Get(), env.Get()) ? 0 : -1;
        } else {
            std::cerr << parser;
            return 1;
        }
    } else {
        return KShim::run(data, argc, argv);
    }
}
