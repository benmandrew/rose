cmake_minimum_required(VERSION 3.20)

project(rose
    VERSION 0.1.0
    DESCRIPTION "Visualise the state spaces of Solitaire games"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(APPEND CMAKE_CXX_FLAGS " -Wall")
string(APPEND CMAKE_CXX_FLAGS " -Wbuiltin-macro-redefined")
string(APPEND CMAKE_CXX_FLAGS " -pedantic")
string(APPEND CMAKE_CXX_FLAGS " -Werror")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified. Defaulting to Debug. To get a release build, run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")

find_program(_CCACHE_PROGRAM NAMES ccache)
if(NOT _CCACHE_PROGRAM)
    if(DEFINED CMAKE_C_COMPILER_LAUNCHER AND CMAKE_C_COMPILER_LAUNCHER)
        message(WARNING "ccache requested as C compiler launcher but not found; unsetting CMAKE_C_COMPILER_LAUNCHER")
        set(CMAKE_C_COMPILER_LAUNCHER "" CACHE STRING "" FORCE)
    endif()
    if(DEFINED CMAKE_CXX_COMPILER_LAUNCHER AND CMAKE_CXX_COMPILER_LAUNCHER)
        message(WARNING "ccache requested as CXX compiler launcher but not found; unsetting CMAKE_CXX_COMPILER_LAUNCHER")
        set(CMAKE_CXX_COMPILER_LAUNCHER "" CACHE STRING "" FORCE)
    endif()
endif()

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

file(GLOB ROSE_LIB_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM ROSE_LIB_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)
add_library(rose_lib STATIC
    ${ROSE_LIB_SRC}
)
target_include_directories(rose_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(rose_lib PUBLIC fmt::fmt nlohmann_json::nlohmann_json)

find_package(fmt CONFIG QUIET)
if (NOT TARGET fmt::fmt)
    include(FetchContent)
    message(STATUS "fmt not found via find_package; fetching fmt via FetchContent")
    FetchContent_Declare(
        fmt
        DOWNLOAD_EXTRACT_TIMESTAMP NEW
        URL https://github.com/fmtlib/fmt/releases/download/12.1.0/fmt-12.1.0.zip)
    FetchContent_MakeAvailable(fmt)
endif()

find_package(json CONFIG QUIET)
if (NOT TARGET nlohmann_json::nlohmann_json)
    include(FetchContent)
    message(STATUS "nlohmann_json not found via find_package; fetching nlohmann_json via FetchContent")
    FetchContent_Declare(
        json
        DOWNLOAD_EXTRACT_TIMESTAMP NEW
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

add_executable(rose src/main.cpp)
target_link_libraries(rose PRIVATE rose_lib)

option(BUILD_WEB "Build the web frontend (requires npm)" ON)

if(BUILD_WEB)
    add_subdirectory(web)
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i --style=file ${ALL_CXX_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-format over source files")
else()
    add_custom_target(format COMMENT "clang-format not found on PATH")
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

# Wire web targets into top-level format/lint/build if available
if(BUILD_WEB)
    if(TARGET format AND TARGET web_format)
        add_dependencies(format web_format)
    endif()
    if(TARGET lint AND TARGET web_lint)
        add_dependencies(lint web_lint)
    endif()
    if(TARGET web_build)
        # Make the main executable depend on the web build so 'cmake --build'
        # will build the web bundle as well.
        add_dependencies(rose web_build)
    endif()
endif()

install(TARGETS rose RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
