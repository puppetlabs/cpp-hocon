include(leatherman)
get_commit_string(PROJECT_COMMIT)

# Generate a file containing the above version numbers.
configure_file("version.h.in" "${CMAKE_CURRENT_LIST_DIR}/inc/hocon/version.h")

# Generate files for Doxygen
configure_file("Doxyfile.in" "${CMAKE_CURRENT_LIST_DIR}/Doxyfile")
configure_file("namespaces.dox.in" "${CMAKE_CURRENT_LIST_DIR}/docs/namespaces.dox")

# Set C++ compile flags based on project settings.
set(CMAKE_CXX_FLAGS ${${PROJECT_NAME_UPPER}_CXX_FLAGS})

# Set prefix for logging messages.
leatherman_logging_namespace("puppetlabs.${PROJECT_NAME}")

# Setup compiling the library.
include_directories(inc ${Boost_INCLUDE_DIRS} ${LEATHERMAN_INCLUDE_DIRS})

set(PROJECT_SOURCES
    src/values/config_object.cc
    src/values/simple_config_object.cc
    src/nodes/abstract_config_node.cc
    src/nodes/config_node_include.cc
    src/nodes/config_node_simple_value.cc
    src/nodes/config_node_single_token.cc
    src/nodes/config_node_comment.cc
    src/nodes/config_node_path.cc
    src/nodes/config_node_complex_value.cc
    src/nodes/config_node_field.cc
    src/nodes/config_node_object.cc
    src/nodes/config_node_array.cc
    src/nodes/config_node_concatenation.cc
    src/nodes/config_node_root.cc
    src/values/config_value.cc
    src/values/config_boolean.cc
    src/values/config_number.cc
    src/values/config_long.cc
    src/values/config_double.cc
    src/values/config_null.cc
    src/values/config_string.cc
    src/values/config_int.cc
    src/values/config_reference.cc
    src/path.cc
    src/path_builder.cc
    src/path_parser.cc
    src/token.cc
    src/tokens.cc
    src/tokenizer.cc
    src/simple_config_document.cc
    src/simple_config_origin.cc
    src/config_util.cc
    src/config_parse_options.cc
    src/config_render_options.cc
    src/config_document_parser.cc
    src/config_document_factory.cc
    src/config_parser.cc
    src/parseable.cc
    src/simple_include_context.cc
    src/simple_includer.cc
    src/config_resolve_options.cc
    src/config.cc
    src/default_transformer.cc
    src/substitution_expression.cc
    src/values/simple_config_list.cc
    src/values/config_delayed_merge.cc
    src/values/config_delayed_merge_object.cc
    src/resolve_context.cc
    src/resolve_source.cc
    src/values/config_concatenation.cc
    )


## An object target is generated that can be used by both the library and test executable targets.
## Without the intermediate target, unexported symbols can't be tested.
add_library(libprojectsrc OBJECT ${PROJECT_SOURCES})
set_target_properties(libprojectsrc PROPERTIES POSITION_INDEPENDENT_CODE true)

add_library(lib${PROJECT_NAME} $<TARGET_OBJECTS:libprojectsrc>)
set_target_properties(lib${PROJECT_NAME} PROPERTIES VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
target_link_libraries(lib${PROJECT_NAME}
    ${LEATHERMAN_LIBRARIES}
    ${Boost_LIBRARIES}
    )

# Generate the export header for restricting symbols exported from the library.
# Restricting symbols has several advantages, noted at https://gcc.gnu.org/wiki/Visibility.
symbol_exports(lib${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/inc/hocon/export.h")

# This correctly handles DLL installation on Windows.
leatherman_install(lib${PROJECT_NAME})
install(DIRECTORY inc/hocon DESTINATION include)

add_subdirectory(tests)
