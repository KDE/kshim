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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#ifndef _WIN32
#include <sys/stat.h>
#endif

using namespace std;

bool KLog::s_doLog = std::getenv("KSHIM_LOG") != nullptr;

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
    if (cmdIt == dataOut.end()) {
        cerr << "Failed to patch binary, please report your compiler" << endl;
        exit(1);
    }

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

bool KShim::createShim(KShimData &shimData, const string &appName, const string &target, const vector<string> &args,  const vector<string> &_env)
{
    vector<pair<string,string>> env;
    env.reserve(_env.size());
    for (const auto &e : _env)
    {
        const auto pos = e.find("=");
        env.push_back({e.substr(0, pos), e.substr(pos + 1)});
    }
    const string outApp = normaliseApplicationName(appName);
    shimData.setApp(target);
    shimData.setArgs(args);
    shimData.setEnv(env);
    const vector<char> binary = readBinary();
    if (!binary.empty()) {
        return writeBinary(outApp, shimData, binary);
    }
    return false;
}

KLog::KLog()
{
}

KLog::~KLog()
{
    if (s_doLog) {
        out() << endl;
    }
}

ofstream &KLog::out()
{
    static ofstream _log;
    if (s_doLog && !_log.is_open()) {
        stringstream logPath;
        logPath << getenv("HOME") << "/.kshim.log";
        _log.open(logPath.str().c_str(), ofstream::app);
        if (!_log.is_open())
        {
            cerr << "KShim: Failed to open log " << logPath.str() << " " << _log.rdstate() << endl;
        }
        _log << "----------------------------" << endl;
    }
    return _log;
}
