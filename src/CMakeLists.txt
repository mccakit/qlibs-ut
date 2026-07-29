add_library(ut)
file(GLOB_RECURSE iface_srcs CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/iface/*.cpp)
file(GLOB_RECURSE impl_srcs CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp)
target_sources(
    ut
    PUBLIC
    FILE_SET CXX_MODULES FILES ${iface_srcs}
    PRIVATE
    FILE_SET SOURCES FILES ${impl_srcs})
target_include_directories(ut PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(ut PROPERTIES EXPORT_NAME "cxx_module")
install(
    TARGETS ut
    EXPORT qlibs-ut-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qlibs-ut)

install(
    PACKAGE_INFO qlibs-ut
    EXPORT qlibs-ut-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cps/qlibs-ut
    CXX_MODULES_DIRECTORY cxxmodules)
