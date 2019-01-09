import subprocess
import shutil
import os

src = [os.path.join(os.path.dirname(__file__), "src", x) for x in ["main.cpp", "kshimdata.cpp", "kshim.cpp"]]

def run( args : [str]) -> int:
    print(" ".join(args))
    return subprocess.run(args)

if shutil.which("cl"):
    run(["cl", "/EHsc", "/O1", "/Fe:kshimgen"] + src)
else:
    cxx = os.environ.get("CXX", "")
    if not cxx:
        if shutil.which("g++"):
            cxx = "g++"
        elif shutil.which("clang++"):
            cxx = "clang++"
    run([cxx, "-O2", "-std=c++14", "-okshimgen"] + src)
