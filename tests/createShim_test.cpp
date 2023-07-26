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

#include "test_config.h"
#include "kshimdata.h"

#include "nlohmann/json.hpp"

namespace {

// dump_args create a json file that we parse
auto readArgs()
{
    nlohmann::json json;
    std::ifstream in;
    in.open("dump_args.json");
    in >> json;
    in.close();

    return json["args"].get<std::vector<std::string>>();
}
}
int main()
{
    KLog::setLoggingEnabled(true);
    KLog::setStdLoggingEnabled(true);

    const auto kshimgen =
            (KShimTest::binaryDir() / KSTRING("kshimgen"s)).replace_extension(KShimLib::exeSuffix);

    const auto dump_args =
            (KShimTest::binaryDir() / KSTRING("dump_args"s)).replace_extension(KShimLib::exeSuffix);


    {
        // test whether the args are correctly passed
        kLog << "Test: args";
        const auto test =
                (std::filesystem::current_path() / KSTRING("test"s)).replace_extension(KShimLib::exeSuffix);


        TEST_EQ(KShimLib::run(KShimData(kshimgen),
                              { KSTRING("--create"s), KSTRING("test"s), dump_args.native() }),
                0);

        TEST_EQ(KShimLib::run(test, { KSTRING("-h"s) }), 0);

        auto args = readArgs();
        TEST_EQ(std::filesystem::path(args[0]).filename(), dump_args.filename());
        TEST_EQ(args[1], "-h");
    }

    {
        // test that arg0 is the name of the test
        kLog << "Test: --keep-arg0";
        const auto test =
                (std::filesystem::current_path() / KSTRING("test2"s)).replace_extension(KShimLib::exeSuffix);

        TEST_EQ(KShimLib::run(KShimData(kshimgen),
                              { KSTRING("--create"s), KSTRING("test2"s), dump_args.native(),
                                KSTRING("--keep-argv0"s) }),
                0);
        TEST_EQ(KShimLib::run(test, { KSTRING("-h"s) }), 0);

        auto args = readArgs();
        TEST_EQ(std::filesystem::path(args[0]), test);
        TEST_EQ(args[1], "-h");
    }

    kLog << "End";
}
