file(GLOB example_sources CONFIGURE_DEPENDS "*.cpp")

foreach(example_source IN LISTS example_sources)
    get_filename_component(example_name "${example_source}" NAME_WE)
    set(example_target "example_${example_name}")
    add_executable(${example_target} "${example_source}")
    target_link_libraries(${example_target} PRIVATE library)
    add_test(NAME ${example_name} COMMAND ${example_target})
endforeach()
