set(CMAKE_CXX_FLAGS ${${PROJECT_NAME_UPPER}_CXX_FLAGS})
leatherman_logging_namespace("puppetlabs.${PROJECT_NAME}.command")

configure_file (
    "driver.cc.in"
    "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}.cc"
)

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

add_test(NAME "smoke_test" COMMAND ${PROJECT_NAME})
add_test(NAME "help_test" COMMAND ${PROJECT_NAME} --help)
add_test(NAME "version_test" COMMAND ${PROJECT_NAME} --version)
