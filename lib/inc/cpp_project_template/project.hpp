/**
 * @file
 * Declares a utility for retrieving the library version.
 */
#pragma once

#include <string>
#include "export.h"

namespace cpp_project_template {

    /**
     * Query the library version.
     * @return A version string with <major>.<minor>.<patch>
     */
    std::string LIBCPP_PROJECT_TEMPLATE_EXPORT version();

}  // namespace cpp_project_template
