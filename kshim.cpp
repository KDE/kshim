#include "kshim.h"

#include "kshimdata.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifdef __APPLE__
#include <libproc.h>
#endif


using namespace std;

namespace  {

string normaliseApplicationName(const string &app)
{
#ifdef _WIN32
    const string exesuffix = ".exe";
    if (app.rfind(exesuffix, app.length() - exesuffix.length()) == string::npos) {
        return string(app).append(exesuffix);
    }
#endif
    return app;
}

vector<char> readBinary()
{
    const auto name = KShim::binaryName();
    ifstream me;
    me.open(name, ios::in | ios::binary);
    if (!me.is_open()) {
        cerr << "Failed to open: " << name << endl;
        return {};
    }

    me.seekg (0, me.end);
    size_t size = static_cast<size_t>(me.tellg());
    me.seekg (0, me.beg);

    vector<char> buf(size);
    me.read(buf.data(), static_cast<streamsize>(size));
    me.close();
    kLog << "Read: " << name << " " << size << " bytes";
    return buf;
}



bool writeBinary(const string &name, const KShimData &shimData, const vector<char> &binary)
{
    vector<char> dataOut = binary;

    // look for the end mark and search for the start from there
    const auto &rawData = shimData.rawData();
    const auto cmdIt = search(dataOut.begin(), dataOut.end(), rawData.cbegin(), rawData.cend());

    ofstream out;
    out.open(name, ios::out | ios::binary);
    if (!out.is_open()) {
        cerr << "Failed to open out: " << name << endl;
        return false;
    }
    const string json = shimData.toJson();
    if (json.size() > rawData.size()) {
        cerr << "Data buffer is too small " << json.size() << " > " << rawData.size() << " :" << endl << json << endl;
        return false;
    }
    copy(json.cbegin(), json.cend(), cmdIt);
    out.write(dataOut.data(), static_cast<streamsize>(binary.size()));
    kLog << "Wrote: " << name << " " << out.tellp() << " bytes";
    out.close();

#ifndef _WIN32
    chmod(name.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    return true;
}
}


string KShim::binaryName()
{
    size_t size;
    string out(1024, 0);
#ifdef _WIN32
    size = GetModuleFileName(nullptr, const_cast<char*>(out.data()), static_cast<DWORD>(out.size()));
#elif __APPLE__
    size = proc_pidpath(getpid(), const_cast<char*>(out.data()), out.size());
#else
    size = readlink("/proc/self/exe", const_cast<char*>(out.data()), out.size());
#endif
    if (size>0) {
        out.resize(size);
    } else {
        cerr << "Failed to locate shimgen" << endl;
        exit(1);
    }
    return out;
}

int KShim::run(const KShimData &data, const vector<string> &args)
{
    auto command = data.formatCommand({args.cbegin() + 1, args.cend()});
    kLog << "#" << command << "#";
    return system(command.c_str());
}


bool KShim::createShim(KShimData &shimData, const vector<string> &args)
{
    if (args.size() < 3) {
        cerr << "Too few arguments" << endl;
        return false;
    }
    const string outApp = normaliseApplicationName(args[2]);
    shimData.setApp(args[3]);
    shimData.setArgs({args.cbegin() + 4, args.cend()});
    const vector<char> binary = readBinary();
    if (!binary.empty()) {
        return writeBinary(outApp, shimData, binary);
    }
    return false;
}

bool KLog::s_doLog = std::getenv("KSHIM_LOG");

KLog::KLog()
{

}

KLog::~KLog()
{
    if (s_doLog) {
        cout << endl;
    }
}
