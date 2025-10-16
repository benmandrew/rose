cmake_minimum_required(VERSION 3.20)

project(solviz
    VERSION 0.1.0
    DESCRIPTION "Visualise the state spaces of Solitaire games"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE _XCRUN_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(_XCRUN_SDK_PATH)
        message(STATUS "Found macOS SDK: ${_XCRUN_SDK_PATH}")
        set(CMAKE_OSX_SYSROOT ${_XCRUN_SDK_PATH})
        string(APPEND CMAKE_C_FLAGS " -isysroot ${_XCRUN_SDK_PATH}")
        string(APPEND CMAKE_CXX_FLAGS " -isysroot ${_XCRUN_SDK_PATH}")
    endif()
endif()

add_library(solviz_lib INTERFACE)
target_include_directories(solviz_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

find_package(fmt CONFIG QUIET)
if (NOT TARGET fmt::fmt)
    include(FetchContent)
    message(STATUS "fmt not found via find_package; fetching fmt via FetchContent")
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.1.1
    )
    FetchContent_MakeAvailable(fmt)
endif()

add_executable(solviz src/main.cpp)
target_link_libraries(solviz PRIVATE solviz_lib fmt::fmt)

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

    add_custom_target(fmt
        COMMAND ${CLANG_FORMAT_EXE} -i --style=file ${ALL_CXX_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format over source files")
else()
    add_custom_target(fmt COMMENT "clang-format not found on PATH")
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-14 clang-tidy-15)
if(CLANG_TIDY_EXE)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy on src files")
else()
    add_custom_target(lint COMMENT "clang-tidy not found on PATH")
endif()

install(TARGETS solviz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Add unit tests
include(CTest)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
