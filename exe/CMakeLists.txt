# Generate source files from a template based on the project name.
# REMOVE after generating your project.
configure_file("driver.cc.in" "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}.cc")
# REMOVE end

# Set C++ compile flags based on project settings.
set(CMAKE_CXX_FLAGS ${${PROJECT_NAME_UPPER}_CXX_FLAGS})

# Set prefix for logging messages.
leatherman_logging_namespace("puppetlabs.${PROJECT_NAME}.command")

# Setup compiling the executable.
include_directories(
    ../lib/inc
    ${Boost_INCLUDE_DIRS}
    ${LEATHERMAN_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cc)
target_link_libraries(${PROJECT_NAME}
    lib${PROJECT_NAME}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${LEATHERMAN_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Tests for the executable. These don't verify behavior, simply that the executable runs
# without crashing or generating an error. Useful, but should be enhanced with test scripts
# and/or acceptance tests.
add_test(NAME "smoke_test" COMMAND ${PROJECT_NAME})
add_test(NAME "help_test" COMMAND ${PROJECT_NAME} --help)
add_test(NAME "version_test" COMMAND ${PROJECT_NAME} --version)
