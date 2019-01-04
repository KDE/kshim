#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifdef __APPLE__
#include <libproc.h>
#endif

namespace {
#define KShimDataDef "KShimData"
static const int BufSize = 1024;

#ifdef _WIN32
static const char DirSep = '\\';
#else
static const char DirSep     = '/';
#endif

struct command
{
    const char cmd[BufSize];
};

static const command StartupCommand {
    KShimDataDef
};
}

namespace KShim
{
std::string binaryName()
{
    size_t size;
    std::string out(BufSize, 0);
#ifdef _WIN32
    size = GetModuleFileName(nullptr, const_cast<char*>(out.data()), BufSize);
#elif __APPLE__
    size = proc_pidpath(getpid(), const_cast<char*>(out.data()), BufSize);
#else
    size = readlink("/proc/self/exe", const_cast<char*>(out.data()), BufSize);
#endif
    if (size>0) {
        out.resize(size);
    } else {
        std::cerr << "Failed to locate shimgen" << std::endl;
        exit(1);
    }
    return out;
}

std::string makeAbsouteCommand(const std::string &_path)
{
    std::string path = _path;
    std::stringstream out;
    if (path[0] == '"')
    {
        out << '"';
        path = path.erase(0, 1);
    }
    if (path[0] == '/' || (path.length() >= 2 && path[1] == ':')) {
        out <<  path;
    } else {
        auto app = binaryName();
        app = app.substr(0, app.rfind(DirSep));
        out << app << DirSep << path;
    }
    return out.str();
}

std::string quote(const std::string &arg)
{
    static std::regex pat("\\s");
    if (std::regex_search(arg, pat))
    {
        std::stringstream out;
        out << std::quoted(arg);
        return out.str();
    }
    return arg;
}

std::string quoteArgs(int argc, char *argv[], int offset=0)
{
    if (argc <= offset)    {
        return {};
    }
    std::stringstream command;
    command << quote(argv[offset]);
    for (int i = offset + 1; i < argc; ++i) {
        command << " " << quote(argv[i]);
    }
    return command.str();
}

std::string normaliseApllication(const std::string &app)
{
#ifdef _WIN32
    const std::string exesuffix = ".exe";
    if (app.rfind(exesuffix, app.length() - exesuffix.length()) == std::string::npos) {
        return std::string(app).append(exesuffix);
    }
#endif
    return app;
}


std::vector<char> readBin(const std::string &name)
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

    std::vector<char> buf(size);
    me.read(buf.data(), static_cast<std::streamsize>(size));
    me.close();
    std::cout << "Read: " << name << " " << size << " bytes" << std::endl;
    return buf;
}

bool writeBin(const std::string &name, const std::string &command, const std::vector<char> &data)
{
    std::vector<char> dataOut = data;

    // look for the end mark and search for the start from there
    const auto cmdIt = std::search(dataOut.begin(), dataOut.end(), StartupCommand.cmd, StartupCommand.cmd + sizeof(StartupCommand.cmd));
    const size_t cmdIndex = static_cast<size_t>(std::distance(dataOut.begin(), cmdIt));

    std::ofstream out;
    out.open(name, std::ios::out | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open out: " << name << std::endl;
        return false;
    }
    std::copy(command.begin(), command.end(), cmdIt);
    dataOut[cmdIndex + command.size()] = 0;
    out.write(dataOut.data(), static_cast<std::streamsize>(data.size()));
    std::cout << "Wrote: " << name << " " << out.tellp() << " bytes" << std::endl;
    out.close();

#ifndef _WIN32
    chmod(name.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return true;
}

int createShim(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "Too few arguments" << std::endl;
        return -1;
    }
    const std::string outApp = normaliseApllication(argv[2]);
    std::stringstream command;
    command << quoteArgs(argc, argv, 3);
    std::cout << outApp << " command: " << command.str() << std::endl;

    const std::vector<char> data = readBin(binaryName());
    if (!data.empty()) {
        return writeBin(outApp, command.str(), data) ? 0 : 1;
    }
    return 1;
}

int run(int argc, char *argv[])
{
    std::stringstream cmd;
    cmd << makeAbsouteCommand(StartupCommand.cmd);
    if (argc >= 2) {
        cmd << " " << quoteArgs(argc, argv, 1);
    }
#ifdef _WIN32
    std::stringstream cmdWin;
    cmdWin << "\"" << cmd.str() << "\"";
    const std::string command = cmdWin.str();
#else
    const std::string command = cmd.str();
#endif
    std::cout << "#" << command << "#" << std::endl;
    return std::system(command.c_str());
}
}

int main(int argc, char *argv[])
{
    if (StartupCommand.cmd == std::string(KShimDataDef)) {
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
