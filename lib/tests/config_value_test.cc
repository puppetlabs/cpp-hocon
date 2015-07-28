#include <catch.hpp>

#include "test_utils.hpp"

using namespace hocon;
using namespace std;

TEST_CASE("simple_config_origin equality", "[config_values]") {
    auto org1 = simple_config_origin("foo");
    auto org2 = simple_config_origin("foo");
    auto org3 = simple_config_origin("bar");

    SECTION("different origins with the same name should be equal") {
        REQUIRE(org1 == org2);
        REQUIRE(org1 != org3);
    }
}

TEST_CASE("config_number equality", "[config_values]") {

    SECTION("config_long equality") {
        config_long a { fake_origin("fake"), 3, "3" };
        config_long a2 { fake_origin("fake"), 3, "3" };
        config_long b { fake_origin("fake"), 4, "4" };

        REQUIRE(a == a2);
        REQUIRE(a != b);
    }

    SECTION("config_double equality") {
        config_double a { fake_origin("fake"), 3.0, "3.0" };
        config_double a2 { fake_origin("fake"), 3.0, "3.0" };
        config_double b { fake_origin("fake"), 4.5, "4.5" };

        REQUIRE(a == a2);
        REQUIRE(a != b);
    }

    SECTION("Int can equal double") {
        config_long an_int { fake_origin("fake"), 3, "3" };
        config_double a_double { fake_origin("fake"), 3.0, "3.0" };
        config_double another_double { fake_origin("fake"), 3.5, "3.5" };

        REQUIRE(an_int == a_double);
        REQUIRE_FALSE(an_int == another_double);
    }
}

TEST_CASE("config numbers ints vs longs vs doubles", "[tokenizer]") {
    SECTION("creates a config_int from an int") {
        unique_ptr<config_number> num = config_number::new_number(fake_origin(), int64_t(2), "2");
        REQUIRE(dynamic_cast<config_int*>(num.get()));
        REQUIRE_FALSE(dynamic_cast<config_long*>(num.get()));
    }

    SECTION("creates config_long from a large long") {
        int64_t definitely_a_long = numeric_limits<int64_t>::max();
        unique_ptr<config_number> num = config_number::new_number(fake_origin(),
            definitely_a_long, to_string(definitely_a_long));
        REQUIRE(dynamic_cast<config_long*>(num.get()));
        REQUIRE_FALSE(dynamic_cast<config_int*>(num.get()));
    }

    SECTION("creates config_int from small whole-number double") {
        unique_ptr<config_number> num = config_number::new_number(fake_origin(), 2.0, "2.0");
        REQUIRE(dynamic_cast<config_int*>(num.get()));
        REQUIRE_FALSE(dynamic_cast<config_double*>(num.get()));
    }

    SECTION("creates config_long from large whole-number double") {
        double big_double = numeric_limits<int>::max() + 1.0;
        unique_ptr<config_number> num = config_number::new_number(
                fake_origin(), big_double, to_string(big_double));
        REQUIRE(dynamic_cast<config_long*>(num.get()));
        REQUIRE_FALSE(dynamic_cast<config_double*>(num.get()));
    }

    SECTION("creates config_double for non-whole number") {
        unique_ptr<config_number> num = config_number::new_number(fake_origin(), 2.5, "2.5");
        REQUIRE(dynamic_cast<config_double*>(num.get()));
        REQUIRE_FALSE(dynamic_cast<config_int*>(num.get()));
    }
}
