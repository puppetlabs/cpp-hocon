#include <cpp_project_template/version.h>
#include <cpp_project_template/project.hpp>

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}

SCENARIO("version() returns the version") {
    REQUIRE(cpp_project_template::version() == CPP_PROJECT_TEMPLATE_VERSION_WITH_COMMIT);
}
