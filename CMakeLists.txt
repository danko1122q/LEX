cmake_minimum_required(VERSION 3.15)
project(lex VERSION 0.5.2 LANGUAGES C)

# Include standard GNU installation directories (bin, lib, share, etc.)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)

# ------------------------------------------------------------------------------
# Bundler Logic (Compiles syntax files into bundle.h)
# ------------------------------------------------------------------------------
set(RESOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/resources")

set(SYNTAX_FILES
    ${RESOURCE_DIR}/syntax/c.json
    ${RESOURCE_DIR}/syntax/cpp.json
    ${RESOURCE_DIR}/syntax/go.json
    ${RESOURCE_DIR}/syntax/html.json
    ${RESOURCE_DIR}/syntax/java.json
    ${RESOURCE_DIR}/syntax/javascript.json
    ${RESOURCE_DIR}/syntax/json.json
    ${RESOURCE_DIR}/syntax/make.json
    ${RESOURCE_DIR}/syntax/php.json
    ${RESOURCE_DIR}/syntax/python.json
    ${RESOURCE_DIR}/syntax/ruby.json
    ${RESOURCE_DIR}/syntax/rust.json
    ${RESOURCE_DIR}/syntax/shell.json
    ${RESOURCE_DIR}/syntax/sql.json
    ${RESOURCE_DIR}/syntax/typescript.json
    ${RESOURCE_DIR}/syntax/zig.json
    ${RESOURCE_DIR}/syntax/markdown.json
)

set(BUNDLER_SOURCE "${RESOURCE_DIR}/bundler.c")
add_executable(bundler ${BUNDLER_SOURCE})
set(BUNDLED_FILE "${RESOURCE_DIR}/bundle.h")

# FIX: Use generator expression for correct bundler path
# and add BYPRODUCTS to ensure bundle.h is always regenerated
add_custom_command(
    OUTPUT ${BUNDLED_FILE}
    COMMAND $<TARGET_FILE:bundler> ${BUNDLED_FILE} ${SYNTAX_FILES}
    DEPENDS bundler ${SYNTAX_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating bundle.h from syntax files..."
    BYPRODUCTS ${BUNDLED_FILE}
)

# Create custom target to ensure bundle.h is always updated
add_custom_target(generate_bundle ALL
    DEPENDS ${BUNDLED_FILE}
)

# ------------------------------------------------------------------------------
# Core Sources
# ------------------------------------------------------------------------------
set(CORE_SOURCES
    src/action.c src/action.h
    src/buildnum.c src/buildnum.h
    src/config.c src/config.h
    src/common.h
    src/editor.c src/editor.h
    src/file_io.c src/file_io.h
    src/highlight.c src/highlight.h
    src/input.c src/input.h
    src/json.h
    src/lex.c
    src/opt.h
    src/os.h
    src/output.c src/output.h
    src/prompt.c src/prompt.h
    src/row.c src/row.h
    src/select.c src/select.h
    src/terminal.c src/terminal.h
    src/unicode.c src/unicode.h
    src/utils.c src/utils.h
)

if (WIN32)
    list(APPEND CORE_SOURCES src/os_win32.c src/os_win32.h)
else()
    list(APPEND CORE_SOURCES src/os_unix.c src/os_unix.h)
endif()

add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${BUNDLED_FILE})

# Ensure lex depends on generate_bundle
add_dependencies(${PROJECT_NAME} generate_bundle)

# ------------------------------------------------------------------------------
# Build Number Logic
# ------------------------------------------------------------------------------
add_custom_target(file_toucher
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/src/buildnum.c
)
add_dependencies(${PROJECT_NAME} file_toucher)

# ------------------------------------------------------------------------------
# Compile Options and Definitions
# ------------------------------------------------------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    EDITOR_NAME="${PROJECT_NAME}"
    EDITOR_VERSION="${CMAKE_PROJECT_VERSION}"
)

set(COMMON_HEADER "${CMAKE_SOURCE_DIR}/src/common.h")
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4244 /wd4267 /wd4996 /FI "${COMMON_HEADER}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -include "${COMMON_HEADER}")
endif()

# ------------------------------------------------------------------------------
# Installation Rules
# ------------------------------------------------------------------------------
# Explicit installation to correct bin directory (system-wide)
# This will install the 'lex' binary to /usr/local/bin (or equivalent)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime
)

# Display actual installation path for easier debugging
message(STATUS "Binary will be installed to: ${CMAKE_INSTALL_FULL_BINDIR}/${PROJECT_NAME}")

# ------------------------------------------------------------------------------
# Custom Targets: UNINSTALL and PURGE
# ------------------------------------------------------------------------------

# More robust and accurate UNINSTALL target
if(NOT TARGET uninstall)
    # Determine exact location of manifest file within build directory
    set(CMAKE_INSTALL_MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
    
    # Configure 'uninstall.cmake.in' template into executable 'uninstall.cmake' script
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        @ONLY
    )
    
    # Add 'uninstall' target that runs the script
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        USES_TERMINAL
        COMMENT "Removing installed files based on install_manifest.txt..."
    )
    message(STATUS "Uninstall target created. Run 'sudo make uninstall' to remove installed files.")
endif()

# FORCE_UNINSTALL target - Explicitly removes binary (without install_manifest.txt)
# This target is useful if install_manifest.txt is missing or corrupted
add_custom_target(force-uninstall
    COMMAND ${CMAKE_COMMAND} -E echo "Force removing binary from install location..."
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_INSTALL_FULL_BINDIR}/${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Checking common installation paths..."
    COMMAND bash -c "sudo rm -f /usr/local/bin/${PROJECT_NAME} || true"
    COMMAND bash -c "sudo rm -f /usr/bin/${PROJECT_NAME} || true"
    COMMAND bash -c "sudo rm -f ${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME} || true"
    COMMAND ${CMAKE_COMMAND} -E echo "Force uninstall complete. Binary '${PROJECT_NAME}' removed from common paths."
    USES_TERMINAL
    COMMENT "Force removing ${PROJECT_NAME} from all common install locations..."
)
message(STATUS "Force-uninstall target created. Run 'sudo make force-uninstall' to forcefully remove the binary.")

# FIXED: CHECK_INSTALL target - Fixed quoting and error handling
add_custom_target(check-install
    COMMAND ${CMAKE_COMMAND} -E echo "Checking for installed '${PROJECT_NAME}' binary..."
    COMMAND bash -c "which '${PROJECT_NAME}' >/dev/null 2>&1 && echo 'Found in PATH: $(which ${PROJECT_NAME})' || echo 'Binary not found in PATH'"
    COMMAND bash -c "[ -f '/usr/local/bin/${PROJECT_NAME}' ] && echo 'Found in /usr/local/bin' || echo 'Not in /usr/local/bin'"
    COMMAND bash -c "[ -f '/usr/bin/${PROJECT_NAME}' ] && echo 'Found in /usr/bin' || echo 'Not in /usr/bin'"
    COMMAND bash -c "[ -f '${CMAKE_INSTALL_FULL_BINDIR}/${PROJECT_NAME}' ] && echo 'Found in ${CMAKE_INSTALL_FULL_BINDIR}' || echo 'Not in ${CMAKE_INSTALL_FULL_BINDIR}'"
    COMMAND ${CMAKE_COMMAND} -E echo "Note: Run 'sudo make install' to install the binary to system"
    USES_TERMINAL
    COMMENT "Checking installation locations for ${PROJECT_NAME}..."
)
message(STATUS "Check-install target created. Run 'make check-install' to verify installation.")

# Improved PURGE target
# The 'purge' target will run 'uninstall' first, before deleting
# the entire build directory. This ensures thorough cleanup.
add_custom_target(purge
    COMMENT "Purging build directory after uninstalling..."
)
# Make 'purge' depend on 'uninstall'
add_dependencies(purge uninstall)
# Command to delete build directory runs after dependencies complete
add_custom_command(TARGET purge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}
)
message(STATUS "Purge target created. Run 'make purge' to uninstall and delete the entire build directory.")

# ------------------------------------------------------------------------------
# Build and Install Information
# ------------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${CMAKE_PROJECT_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary install dir: ${CMAKE_INSTALL_FULL_BINDIR}")
message(STATUS "")
message(STATUS "=== Available Targets ===")
message(STATUS "  make              - Build the project")
message(STATUS "  sudo make install - Install to ${CMAKE_INSTALL_FULL_BINDIR}")
message(STATUS "  make check-install - Check where binary is installed")
message(STATUS "  sudo make uninstall - Remove installed files (requires install_manifest.txt)")
message(STATUS "  sudo make force-uninstall - Force remove binary from common paths")
message(STATUS "  make purge        - Uninstall and delete build directory")
message(STATUS "==========================")