set(CMAKE_CXX_FLAGS ${CPP_PROJECT_TEMPLATE_CXX_FLAGS})
leatherman_logging_namespace("puppetlabs.cpp_project_template.driver")

include_directories(
    ../lib/inc
    ${Boost_INCLUDE_DIRS}
    ${LEATHERMAN_INCLUDE_DIRS}
)

add_executable(driver driver.cc)
target_link_libraries(driver libcpp_project_template ${Boost_PROGRAM_OPTIONS_LIBRARY} ${LEATHERMAN_LIBRARIES})
install(TARGETS driver DESTINATION bin)

add_test(NAME "smoke_test" COMMAND driver)
add_test(NAME "help_test" COMMAND driver --help)
add_test(NAME "version_test" COMMAND driver --version)
