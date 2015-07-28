# Setup compiling the test executable. C++ compile flags are inherited from the parent directory.
include_directories(${LEATHERMAN_CATCH_INCLUDE})

set(TEST_CASES
    config_value_test.cc
)

add_executable(lib${PROJECT_NAME}_test $<TARGET_OBJECTS:libprojectsrc> ${TEST_CASES} main.cc)
target_link_libraries(lib${PROJECT_NAME}_test
    ${Boost_LIBRARIES}
    ${LEATHERMAN_LIBRARIES}
)

add_test(NAME "unit_tests" COMMAND lib${PROJECT_NAME}_test)
