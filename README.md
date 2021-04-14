<h1 align="center">Tiny SQL</h1>

<p align='center'>Shabby implementation of relational database.</p>

# Prerequisite

- **CMake >= 3.14**: https://cmake.org/download/
- **Python >= 3.3**: https://www.python.org/downloads/(on UNIX like system such as **Linux or Mac** we have Python pre-installed. So there is no need to install Python if you use one of them)



# Installation

We carefully prepared **cross-platform installation script**, which hide the complex underlying details of the common building workflow powered by CMake.

```bash
git clone git@github.com:ShZh-Playground/tiny-SQL.git

# Linux or Mac
./script/install
# Windows
python ./script/install.py
```

Then you will get a executable file named `tinyDB` (on **Unix-like system**)or `tinyDB.exe`(on **Windows**) in your generated `./bin` directory. Type it in your shell to start a tinyDB process.



# License

[MIT License](LICENSE)

Copyright (c) 2021 sh-zh-7