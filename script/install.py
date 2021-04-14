#!/usr/bin/python3
import os
import shutil

# Change your default build type here
cmake_build_type = "Release"

if __name__ == "__main__":
    # Configure
    os.system("cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=" + cmake_build_type)
    # Build
    os.system("cmake --build build --config " + cmake_build_type)

    if (os.path.exists("bin")):
        # Windows target path: .\bin\Debug\xxx.exe
        # Move them to .\bin\xxx.exe
        if (os.name == "nt"):
            for root, _, files in os.walk("bin"):
                for f in files:
                    src = os.path.join(root, f)
                    dst = os.path.join("bin", f)
                    shutil.move(src, dst)
                    os.rmdir(os.path.join("bin", cmake_build_type))
    else:
        print("Error building!")
