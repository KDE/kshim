import subprocess
import shutil

src = ["main.cpp", "kshimdata.cpp", "kshim.cpp"]

def run( args : [str]) -> int:
    print(" ".join(args))
    return subprocess.run(args)

if shutil.which("cl"):
    run(["cl", "/EHsc", "/O1", "/Fe:kshimgen"] + src)
elif shutil.which("g++"):
    run(["g++", "-O2", "-okshimgen"] + src)
elif shutil.which("clang++"):
    run(["clang++", "-O2", "-okshimgen"] + src)
