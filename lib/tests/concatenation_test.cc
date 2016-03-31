#include <catch.hpp>

#include <hocon/config.hpp>
#include "test_utils.hpp"

using namespace hocon;

TEST_CASE("concatenation") {

    SECTION("no substitutions string concatenation") {
        auto conf = parse_config(R"(a :  true "xyz" 123 foo)")->resolve();
        REQUIRE(conf->get_string("a") == "true xyz 123 foo");
    }

    SECTION("trivial string concatenation") {
        auto conf = parse_config(R"(a : ${x}foo, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1foo");
    }

    SECTION("variable lookup o:") {
        auto conf = parse_config(R"(a : ${x}, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1");
    }

}
