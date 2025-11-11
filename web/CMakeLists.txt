cmake_minimum_required(VERSION 3.20)

project(rose_web NONE)

find_program(NPM_EXECUTABLE npm)

if(NPM_EXECUTABLE)
    message(STATUS "Found npm: ${NPM_EXECUTABLE}")

    add_custom_target(web_install_deps
        COMMAND ${NPM_EXECUTABLE} ci
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Installing web dependencies (npm ci)")

    add_custom_target(web_build
        COMMAND ${NPM_EXECUTABLE} run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building web bundle"
        DEPENDS web_install_deps)

    add_custom_target(web_format
        COMMAND ${NPM_EXECUTABLE} run format
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting web sources with biome")

    add_custom_target(web_format_check
        COMMAND ${NPM_EXECUTABLE} run format-check
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking web formatting (biome)")

    add_custom_target(web_lint
        COMMAND ${NPM_EXECUTABLE} run lint
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Linting web sources (biome)")
else()
    message(WARNING "npm not found; web build/format/lint targets will be no-ops")
    add_custom_target(web_install_deps COMMENT "npm not found; skipping web_install_deps")
    add_custom_target(web_build COMMENT "npm not found; skipping web_build")
    add_custom_target(web_format COMMENT "npm not found; skipping web_format")
    add_custom_target(web_format_check COMMENT "npm not found; skipping web_format_check")
    add_custom_target(web_lint COMMENT "npm not found; skipping web_lint")
endif()
