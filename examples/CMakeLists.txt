add_executable(assertions)
target_sources(
    assertions
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/assertions.cpp)
target_link_libraries(assertions PRIVATE ut)

add_executable(custom_config)
target_sources(
    custom_config
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/custom_config.cpp)
target_link_libraries(custom_config PRIVATE ut)

add_executable(diagnostics)
target_sources(
    diagnostics
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/diagnostics.cpp)
target_link_libraries(diagnostics PRIVATE ut)

add_executable(execution_model)
target_sources(
    execution_model
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/execution_model.cpp)
target_link_libraries(execution_model PRIVATE ut)

add_executable(fatal)
target_sources(
    fatal
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/fatal.cpp)
target_link_libraries(fatal PRIVATE ut)

add_executable(filtering)
target_sources(
    filtering
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/filtering.cpp)
target_link_libraries(filtering PRIVATE ut)

add_executable(hello_world)
target_sources(
    hello_world
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/hello_world.cpp)
target_link_libraries(hello_world PRIVATE ut)

add_executable(suites_and_subtests)
target_sources(
    suites_and_subtests
    PRIVATE
    FILE_SET SOURCES FILES ${CMAKE_CURRENT_SOURCE_DIR}/suites_and_subtests.cpp)
target_link_libraries(suites_and_subtests PRIVATE ut)
