#include <catch.hpp>

#include <internal/simple_config_origin.hpp>
#include <internal/config_int.hpp>
#include <internal/config_double.hpp>

using namespace hocon;

TEST_CASE("simple_config_origin equality", "[config_values]") {
    auto org1 = *simple_config_origin::new_simple("foo");
    auto org2 = *simple_config_origin::new_simple("foo");
    auto org3 = *simple_config_origin::new_simple("bar");

    SECTION("different origins with the same name should be equal") {
        REQUIRE(org1 == org2);
        REQUIRE_FALSE(org1 != org2);
        REQUIRE(org1 != org3);
    }
}

TEST_CASE("config_number equality", "[config_values]") {

    SECTION("config_int equality") {
        config_int a { simple_config_origin::new_ };
    }
}
