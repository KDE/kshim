#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <tuple>

namespace {
#define KShimStart "KShimStart"

#ifdef _WIN32
static const char QuoteDelim = '"';
#else
static const char QuoteDelim = '\'';
#endif

struct command
{
    const char cmd[256*2];
};

static const command StartupCommand {
    KShimStart
};
}

namespace KShim
{
std::string quote(int argc, char *argv[], int offset=0)
{
    if (argc <= offset)    {
        return {};
    }
    std::stringstream command;
    command << std::quoted(argv[offset], QuoteDelim);
    for (int i = offset + 1; i < argc; ++i) {
        command << " " << std::quoted(argv[i], QuoteDelim);
    }
    return command.str();
}

std::string normaliseApllication(const std::string &app)
{
#ifdef _WIN32
    if (app.rfind(".exe", 0) == std::string::npos) {
        return std::string(app).append(".exe");
    }
#endif
    return app;
}


std::tuple<char *, size_t> readBin(const std::string &name)
{
    std::ifstream me;
    me.open(name, std::ios::in | std::ios::binary);
    if (!me.is_open()) {
        std::cout << "Failed to open: " << name << std::endl;
        return {};
    }

    me.seekg (0, me.end);
    size_t size = static_cast<size_t>(me.tellg());
    me.seekg (0, me.beg);

    char *buf = new char[size];
    me.read(buf, static_cast<std::streamsize>(size));
    me.close();
    return {buf, size};
}

bool writeBin(const std::string &name, const std::string &command, char *buf, size_t size)
{
    std::string pattern(KShimStart);
    pattern.resize(sizeof(StartupCommand.cmd));
    auto start = std::search(buf, buf + size, pattern.data(), pattern.data() + pattern.size());

    std::ofstream out;
    out.open(name, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        std::cout << "Failed to open out: " << name << std::endl;
        return false;
    }
    out.write(buf, start - buf);
    out << command;

    auto pos = start + command.size();
    out.write(pos, buf + size - pos);

    out.close();
    return true;
}

int createShim(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "Too few arguments" << std::endl;
        return -1;
    }
    const std::string app = normaliseApllication(argv[0]);
    const std::string outApp = normaliseApllication(argv[2]);
    const std::string command = quote(argc, argv, 3);
    std::cout << outApp << " command: " << command << std::endl;

    char *buf;
    size_t size;
    std::tie(buf, size) = readBin(app);
    if (size > 0) {
        bool out = writeBin(outApp, command, buf, size);
        delete [] buf;
        return 0;
    }
    return 1;
}

int run(int argc, char *argv[])
{
    std::stringstream cmd;
#ifdef _WIN32
    cmd << "\"";
#endif
    cmd << StartupCommand.cmd;
    cmd  << quote(argc, argv, 1);
#ifdef _WIN32
    cmd << "\"";
#endif
    std::cout << cmd.str() << std::endl;
    return std::system(cmd.str().c_str());
}
}

int main(int argc, char *argv[])
{
    if (StartupCommand.cmd == std::string(KShimStart)) {
        if (argc > 1) {
            const std::string arg1 = argv[1];
            if (arg1 == "--create") {
                return KShim::createShim(argc, argv);
            }
        } else {
            std::cout << "Usage: --create targe command" << std::endl;
        }
        return -1;
    } else {
        return KShim::run(argc, argv);
    }
}
