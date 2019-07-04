import subprocess
import shutil
import shlex
import os


srcDir = os.path.dirname(__file__)
includes = ["-I" + os.path.join(srcDir, "src", "3dparty", x) for x in ["nlohmann"]]
srcFiles = ["kshimdata.cpp", "kshim.cpp", "kshimpath.cpp"]

if os.name == 'nt':
    srcFiles += ["kshim_win.cpp"]
else:
    srcFiles += ["kshim_unix.cpp"]

src = [os.path.join(srcDir, "src", x) for x in srcFiles]


def run( args : [str]) -> int:
    print(" ".join(args))
    return subprocess.run(args)

if shutil.which("cl"):
    src += [os.path.join(srcDir, "src", "main_win.cpp")]
    run(["cl", "/EHsc", "/O1", "/Fe:kshimgen", "-D_KSHIM_BOOTSTRAP", "-DUNICODE", "-D_UNICODE", "shell32.lib"] + src + includes)
else:
    cxx = os.environ.get("CXX", "")
    if cxx:
        cxx = shlex.split(cxx)
    else:
        if shutil.which("g++"):
            cxx = ["g++"]
        elif shutil.which("clang++"):
            cxx = ["clang++"]
    src += [os.path.join(srcDir, "src", "main_unix.cpp")]
    run(cxx + ["-O2", "-std=c++11", "-okshimgen", "-D_KSHIM_BOOTSTRAP"] + src + includes)
