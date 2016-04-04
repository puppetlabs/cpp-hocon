#include <catch.hpp>

#include <hocon/config.hpp>
#include "test_utils.hpp"

using namespace hocon;
using namespace hocon::test_utils;

TEST_CASE("concatenation") {

    SECTION("no substitutions string concatenation") {
        auto conf = parse_config(R"(a :  true "xyz" 123 foo)")->resolve();
        REQUIRE(conf->get_string("a") == "true xyz 123 foo");
    }
}

// These tests are expected to fail because resolve_result::resolve implementation is incomplete.
TEST_CASE("concatenation pending", "[!shouldfail]") {
    SECTION("trivial string concatenation") {
        auto conf = parse_config(R"(a : ${x}foo, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1foo");
    }

    SECTION("variable lookup o:") {
        auto conf = parse_config(R"(a : ${x}, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1");
    }

}
