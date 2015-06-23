#include <cpp_project_template/version.h>
#include <cpp_project_template/project.hpp>

#include <leatherman/logging/logging.hpp>

namespace cpp_project_template {

    using namespace std;

    string version()
    {
        LOG_DEBUG("Cpp-project-template version is %1%", CPP_PROJECT_TEMPLATE_VERSION_WITH_COMMIT);
        return CPP_PROJECT_TEMPLATE_VERSION_WITH_COMMIT;
    }

}  // cpp_project_template
