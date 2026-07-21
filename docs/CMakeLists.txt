 add_custom_target(
    doc
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/doxygen doxygen Doxyfile > /dev/null 2>&1
)
