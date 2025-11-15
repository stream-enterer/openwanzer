# ==============================================================================
# Open Wanzer - CMake Configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(OpenWanzer VERSION 0.1.0 LANGUAGES CXX)

# ==============================================================================
# C++ Standard
# ==============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Build Type
# ==============================================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ==============================================================================
# Output Directories
# ==============================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==============================================================================
# Compiler Flags
# ==============================================================================

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# ==============================================================================
# Include Directories
# ==============================================================================

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# ==============================================================================
# Source Files
# ==============================================================================

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ==============================================================================
# raylib Configuration
# ==============================================================================

# Add raylib library directory
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Set rpath for runtime library loading
set(CMAKE_BUILD_RPATH ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_INSTALL_RPATH ${CMAKE_SOURCE_DIR}/lib)

# ==============================================================================
# Executable Target
# ==============================================================================

add_executable(openwanzer ${SOURCES})

# ==============================================================================
# Link Libraries
# ==============================================================================

target_link_libraries(openwanzer
    raylib
    m
    pthread
    dl
    rt
    X11
)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS openwanzer
    RUNTIME DESTINATION bin
)

# ==============================================================================
# Custom Targets
# ==============================================================================

# Format target (requires clang-format)
add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting code with clang-format"
)

# Run target
add_custom_target(run
    COMMAND openwanzer
    DEPENDS openwanzer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Open Wanzer"
)

# Check dependencies target
add_custom_target(check-deps
    COMMAND test -f ${CMAKE_SOURCE_DIR}/lib/libraylib.a -o -f ${CMAKE_SOURCE_DIR}/lib/libraylib.so || (echo "✗ raylib not found in ./lib/" && exit 1)
    COMMAND test -f ${CMAKE_SOURCE_DIR}/include/rl/raylib.h || (echo "✗ raylib.h not found in ./include/rl/" && exit 1)
    COMMAND test -f ${CMAKE_SOURCE_DIR}/include/rl/raygui.h || (echo "✗ raygui.h not found in ./include/rl/" && exit 1)
    COMMAND echo "✓ All dependencies found"
    COMMENT "Checking dependencies"
)

# ==============================================================================
# Build Information
# ==============================================================================

message(STATUS "OpenWanzer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "  Source Files: ${CMAKE_SOURCE_DIR}/src/*.cpp")
