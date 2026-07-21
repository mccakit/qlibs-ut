### Documentation

https://boost-ext.github.io/ut/

### API Reference

https://mccakit.github.io/boost_ut/

### Packaging and Consumption

This library is packaged by CPS, to consume:
```cmake
find_package(ut REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ut::ut)
```
