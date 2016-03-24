# Defines how cmake should behave, and the minimum version necessary to build.
cmake_minimum_required(VERSION 3.2.2)

# Project Setup - modify to match project naming
## Source code for a simple command-line executable for a dynamic library will be generated from the project name.
## The command-line and library names will be based off the project name.
project(cpp-hocon VERSION 0.1.0)

string(MAKE_C_IDENTIFIER ${PROJECT_NAME} PROJECT_C_NAME)
string(TOUPPER ${PROJECT_C_NAME} PROJECT_NAME_UPPER)
string(TOLOWER ${PROJECT_C_NAME} PROJECT_NAME_LOWER)

# Common cmake setup
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Defaulting to a release build.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

enable_testing()

# Leatherman setup
set(LEATHERMAN_COMPONENTS catch nowide)
find_package(Leatherman REQUIRED COMPONENTS ${LEATHERMAN_COMPONENTS})

## Before we find any packages, we want to pull in the common leatherman options, as they can affect commonly-used packages.
include(options)
## Pull in common cflags setting from leatherman. Don't override CMAKE_CXX_FLAGS at the project root to avoid impacting 3rd party code.
include(cflags)
set(${PROJECT_NAME_UPPER}_CXX_FLAGS "${LEATHERMAN_CXX_FLAGS}")
add_definitions(${LEATHERMAN_DEFINITIONS})
## Pull in helper macros for working with leatherman libraries
include(leatherman)

# Add other dependencies
find_package(Boost 1.54 REQUIRED COMPONENTS program_options)

# Display a summary of the features
include(FeatureSummary)
feature_summary(WHAT ALL)

add_subdirectory(lib)

# Add cpplint and cppcheck targets
file(GLOB_RECURSE ALL_SOURCES lib/src/*.cc lib/inc/*.hpp exe/*.cc)
add_cpplint_files(${ALL_SOURCES})
enable_cpplint()

add_cppcheck_dirs("${PROJECT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/exe")
enable_cppcheck()

