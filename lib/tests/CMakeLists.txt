include_directories(${LEATHERMAN_CATCH_INCLUDE})

configure_file (
    "test.cc.in"
    "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}.cc"
)

set(TEST_CASES ${PROJECT_NAME}.cc)

add_executable(lib${PROJECT_NAME}_test $<TARGET_OBJECTS:libprojectsrc> ${TEST_CASES} main.cc)
target_link_libraries(lib${PROJECT_NAME}_test
    ${Boost_LIBRARIES}
    ${LEATHERMAN_LIBRARIES}
)

add_test(NAME "unit_tests" COMMAND lib${PROJECT_NAME}_test)
