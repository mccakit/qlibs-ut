add_library(ut)
set_target_properties(ut PROPERTIES EXPORT_NAME "library")

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(ut PUBLIC FILE_SET CXX_MODULES FILES ${srcs})
target_include_directories(ut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
install(
    TARGETS ut
    EXPORT ut-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ut)

install(
    PACKAGE_INFO ut
    EXPORT ut-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cps/ut
    CXX_MODULES_DIRECTORY cxxmodules)
