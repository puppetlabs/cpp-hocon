# Setup compiling the test executable. C++ compile flags are inherited from the parent directory.
include_directories(${LEATHERMAN_CATCH_INCLUDE})

set(TEST_CASES
    concatenation_test.cc
    config_document_parser_test.cc
    config_node_test.cc
    config_value_test.cc
    path_test.cc
    token_test.cc
    tokenizer_test.cc
    test_utils.cc
    config_document_tests.cc
    conf_parser_test.cc
    config_substitution_test.cc
    config_value_factory_test.cc
    config_test.cc
    program_options.cc
)

add_executable(lib${PROJECT_NAME}_test $<TARGET_OBJECTS:libprojectsrc> ${TEST_CASES} main.cc)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    target_link_libraries(lib${PROJECT_NAME}_test rt)
endif()

target_link_libraries(lib${PROJECT_NAME}_test
    ${Boost_LIBRARIES}
    ${LEATHERMAN_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND BOOST_STATIC AND LEATHERMAN_USE_LOCALES)
    target_link_libraries(lib${PROJECT_NAME}_test iconv)
endif()

add_test(NAME "unit_tests" COMMAND lib${PROJECT_NAME}_test)

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/fixtures.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/fixtures.hpp"
)
