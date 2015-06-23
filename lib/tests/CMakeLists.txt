include_directories(${LEATHERMAN_CATCH_INCLUDE})

add_executable(libproject_test $<TARGET_OBJECTS:libprojectsrc> test.cc)
target_link_libraries(libproject_test
    ${Boost_LIBRARIES}
    ${LEATHERMAN_LIBRARIES}
)

add_test(NAME "unit_tests" COMMAND libproject_test)
