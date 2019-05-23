import subprocess
import shutil
import shlex
import os


srcDir = os.path.dirname(__file__)
includes = ["-I" + os.path.join(srcDir, "src", "3dparty", x) for x in ["args", "nlohmann"]]
srcFiles = ["main.cpp", "kshimdata.cpp", "kshim.cpp"]

if os.name == 'nt':
    srcFiles += ["kshim_win.cpp"]
else:
    srcFiles += ["kshim_unix.cpp"]

src = [os.path.join(srcDir, "src", x) for x in srcFiles]


def run( args : [str]) -> int:
    print(" ".join(args))
    return subprocess.run(args)

if shutil.which("cl"):
    run(["cl", "/EHsc", "/O1", "/Fe:kshimgen"] + src + includes)
else:
    cxx = os.environ.get("CXX", "")
    if cxx:
        cxx = shlex.split(cxx)
    else:
        if shutil.which("g++"):
            cxx = ["g++"]
        elif shutil.which("clang++"):
            cxx = ["clang++"]
    run(cxx + ["-O2", "-std=c++11", "-okshimgen"] + src + includes)
