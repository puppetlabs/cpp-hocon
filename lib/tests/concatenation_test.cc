#include <catch.hpp>

#include <hocon/config.hpp>

using namespace hocon;

TEST_CASE("concatenation") {

    SECTION("no substitutions") {
        auto conf = config::parse_string(R"( a :  true "xyz" 123 foo  )")->resolve();
        REQUIRE(conf->get_string("a") == "true xyz 123 foo");
    }
}
