#include "kshimdata.h"
#include "kshim.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <sstream>

#define KShimDataDef "KShimData"

using namespace std;
using namespace nlohmann;

namespace  {
static const int DataStorageSize = 1024 * 2;

struct command
{
    const char cmd[DataStorageSize];
};

static const command StartupCommand {
    KShimDataDef
};
}

KShimData::KShimData()
    : m_rawData(StartupCommand.cmd, StartupCommand.cmd +  sizeof(StartupCommand.cmd))
{
    if (isShim())
    {
        json data = json::parse(m_rawData);
        m_app = data["app"].get<string>();
        m_args = data["args"].get<vector<string>>();
    }
}

string KShimData::app() const
{
    return m_app;
}

void KShimData::setApp(const string &app)
{
    m_app = app;
}

const vector<string> &KShimData::args() const
{
    return m_args;
}

void KShimData::setArgs(const vector<string> &args)
{
    m_args = args;
}

string KShimData::formatCommand(const vector<string> &arguments) const
{
#ifdef _WIN32
    const string extraQuote = "\"";
#else
    const string extraQuote = "";
#endif
    stringstream cmd;
    cmd << extraQuote
        << makeAbsouteCommand(app())
        << quoteArgs(args())
        << quoteArgs(arguments)
        << extraQuote;
    return cmd.str();
}

const vector<char> &KShimData::rawData() const
{
    return m_rawData;
}

string KShimData::toJson() const
{
    return json{
        {"app", app()},
        {"args", args()},
    }.dump();
}

bool KShimData::isShim() const
{
    return StartupCommand.cmd != string(KShimDataDef);
}


string KShimData::makeAbsouteCommand(const string &_path) const
{
    string path = _path;
    stringstream out;
    if (path[0] == '"')
    {
        out << '"';
        path = path.erase(0, 1);
    }
    if (path[0] == '/' || (path.length() >= 2 && path[1] == ':')) {
        out <<  path;
    } else {
        auto app = KShim::binaryName();
        app = app.substr(0, app.rfind(KShim::dirSep()));
        out << app << KShim::dirSep() << path;
    }
    return quote(out.str());
}

string KShimData::quote(const string &arg) const
{
    static regex pat("\\s");
    if (regex_search(arg, pat))
    {
        stringstream out;
        out << quoted(arg);
        return out.str();
    }
    return arg;
}

string KShimData::quoteArgs(vector<string> args) const
{
    stringstream command;
    for (const auto &arg : args) {
        command << " " << quote(arg);
    }
    return command.str();
}
