cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(catch2)

add_executable(unittests
    test_common.cpp
    test_table.cpp
    test_moves.cpp
)
target_link_libraries(unittests PRIVATE Catch2::Catch2WithMain rose_lib)
target_include_directories(unittests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TEST_RES_DIR "${CMAKE_CURRENT_BINARY_DIR}/res")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/res_config.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/res_config.hpp @ONLY)
target_include_directories(unittests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/res)
    message(FATAL_ERROR "No tests/res/ directory found in source tree")
endif()

add_custom_target(copy_test_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMENT "Copying test resources to ${CMAKE_CURRENT_BINARY_DIR}/res"
)
add_dependencies(unittests copy_test_resources)

include(Catch)

catch_discover_tests(unittests)

add_custom_target(tests
    DEPENDS unittests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
