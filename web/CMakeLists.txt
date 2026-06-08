cmake_minimum_required(VERSION 3.20)

project(rose_web NONE)

find_program(NPM_EXECUTABLE npm)

if(NPM_EXECUTABLE)
    message(STATUS "Found npm: ${NPM_EXECUTABLE}")

    set(NPM_STAMP "${CMAKE_CURRENT_SOURCE_DIR}/node_modules/.npm-ci.stamp")
    add_custom_command(
        OUTPUT ${NPM_STAMP}
        COMMAND ${NPM_EXECUTABLE} ci --quiet --no-fund
        COMMAND ${CMAKE_COMMAND} -E touch ${NPM_STAMP}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json
            ${CMAKE_CURRENT_SOURCE_DIR}/package.json
        COMMENT "Installing web dependencies (npm ci)")

    add_custom_target(web_install_deps
        DEPENDS ${NPM_STAMP})

    add_custom_target(web_build
        COMMAND ${NPM_EXECUTABLE} run build --quiet
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
    set(SITE_EXAMPLES_PATH "/assets/rose/examples"
        CACHE STRING "Base URL for example JSON files in the site build")

    set(SITE_DIST_DIR ${CMAKE_CURRENT_BINARY_DIR}/site)

    add_custom_target(site
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SITE_DIST_DIR}
        COMMAND ${NPM_EXECUTABLE} run --quiet build --
            --env examplesPath=${SITE_EXAMPLES_PATH}
            --env outputPath=${SITE_DIST_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building site assets"
        DEPENDS web_install_deps)

else()
    message(WARNING "npm not found; web build/format/lint targets will be no-ops")
    add_custom_target(web_install_deps COMMENT "npm not found; skipping web_install_deps")
    add_custom_target(web_build COMMENT "npm not found; skipping web_build")
    add_custom_target(web_format COMMENT "npm not found; skipping web_format")
    add_custom_target(web_format_check COMMENT "npm not found; skipping web_format_check")
    add_custom_target(web_lint COMMENT "npm not found; skipping web_lint")
    add_custom_target(site COMMENT "npm not found; skipping site")
endif()
