set(CPP_PROJECT_TEMPLATE_VERSION_MAJOR 0)
set(CPP_PROJECT_TEMPLATE_VERSION_MINOR 0)
set(CPP_PROJECT_TEMPLATE_VERSION_PATCH 0)

# Generate a file containing the above version numbers
configure_file (
    "version.h.in"
    "${CMAKE_CURRENT_LIST_DIR}/inc/cpp_project_template/version.h"
)

set(CMAKE_CXX_FLAGS ${CPP_PROJECT_TEMPLATE_CXX_FLAGS})
leatherman_logging_namespace("puppetlabs.cpp_project_template")

include_directories(inc ${Boost_INCLUDE_DIRS} ${LEATHERMAN_INCLUDE_DIRS})

set(PROJECT_SOURCES "src/project.cc")

# An object target is used to declare
add_library(libprojectsrc OBJECT ${PROJECT_SOURCES})
set_target_properties(libprojectsrc PROPERTIES POSITION_INDEPENDENT_CODE true)

add_library(libcpp_project_template SHARED $<TARGET_OBJECTS:libprojectsrc>)
set_target_properties(libcpp_project_template PROPERTIES PREFIX "" SUFFIX ".so" IMPORT_PREFIX "" IMPORT_SUFFIX ".so.a" VERSION "${CPP_PROJECT_TEMPLATE_VERSION_MAJOR}.${CPP_PROJECT_TEMPLATE_VERSION_MINOR}.${CPP_PROJECT_TEMPLATE_VERSION_PATCH}")
target_link_libraries(libcpp_project_template PRIVATE
    ${LEATHERMAN_LIBRARIES}
    ${Boost_LIBRARIES}
)

# Generate the export header
include(GenerateExportHeader)
generate_export_header(libcpp_project_template EXPORT_FILE_NAME "${CMAKE_CURRENT_LIST_DIR}/inc/cpp_project_template/export.h")
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_compiler_export_flags()
elseif (WIN32)
    add_compiler_export_flags()
    add_definitions("-Dlibcpp_project_template_EXPORTS")
endif()

get_filename_component(LIBPATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} NAME)
install(TARGETS libcpp_project_template DESTINATION ${LIBPATH})
install(DIRECTORY inc/cpp_project_template DESTINATION include)

add_subdirectory(tests)
