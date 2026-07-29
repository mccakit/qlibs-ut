# qlibs-ut

User Manual | Api Reference

qlibs-ut is a C++ Module port of [qlibs-ut](https://github.com/qlibs/ut), C++20 Unit-Testing library

Project is built using CMake/Ninja and packaged via CPS. CMake 4.4 and later is required.

Build using cmake, and consume via CPS by pointing to `CMAKE_INSTALL_PREFIX` via `CMAKE_PREFIX_PATH`

```cmake
find_package(qlibs-ut)
target_link_libraries($PROJECT PRIVATE qlibs-ut::cxx_module)
```

