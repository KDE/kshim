#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

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

  static const command StartupCommand
  {
    KShimStart
  };
}

namespace KShim
{
  std::string quote(int argc, char *argv[], int offset=0)
  {
    if (argc <= offset)
    {
      return {};
    }
    std::stringstream command;
    command << std::quoted(argv[offset], QuoteDelim);
    for (int i = offset + 1; i < argc; ++i)
    {
      command << " " << std::quoted(argv[i], QuoteDelim);
    }
    return command.str();
  }

  std::string normaliseApllication(const std::string &app)
  {
#ifdef _WIN32
    if (app.rfind(".exe", 0) == std::string::npos)
    {
        return std::string(app).append(".exe");
    }
#endif
    return app;
  }

  int createShim(int argc, char *argv[])
  {
    if (argc < 3)
    {
      std::cerr << "Too feew arguments" << std::endl;
      return -1;
    }
    const std::string app = normaliseApllication(argv[0]);
    const std::string outApp = normaliseApllication(argv[2]);
    const std::string command = quote(argc, argv, 3);

    std::ofstream out;
    out.open(outApp.c_str(), std::ios::out | std::ios::binary);
    std::ifstream me;
    me.open(app, std::ios::in | std::ios::binary);
    if (!me.is_open())
    {
      std::cerr << "Failed to open: " << app << std::endl;
      return -1;
    }
    if (!out.is_open())
    {
      std::cerr << "Failed to open out: " << app << std::endl;
      return -1;
    }

    me.seekg (0, me.end);
    const size_t size = static_cast<size_t>(me.tellg());
    me.seekg (0, me.beg);

    char *buf = new char[size];
    me.read(buf, size);

    const std::string pattern(KShimStart);
    auto start = std::search(buf, buf + size, pattern.data(), pattern.data() + pattern.size());
    out.write(buf, start - buf);
    out << command;

    auto pos = start + command.size();
    out.write(pos, buf + size - pos);

    me.close();
    out.close();
    delete [] buf;
    return 0;
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
  if (StartupCommand.cmd == std::string(KShimStart))
  {
    if (argc > 1)
    {
      const std::string arg1 = argv[1];
      if (arg1 == "--create")
      {
        return KShim::createShim(argc, argv);
      }
    }
    return -1;
  }
  else
  {
    return KShim::run(argc, argv);
  }
}
