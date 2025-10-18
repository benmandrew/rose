cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(catch2)

add_executable(test
    test_common.cpp
    test_table.cpp
    test_moves.cpp
)
target_link_libraries(test PRIVATE Catch2::Catch2WithMain solviz_lib)
target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

include(CTest)
include(Catch)

catch_discover_tests(test)

add_custom_target(tests
    DEPENDS test
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
