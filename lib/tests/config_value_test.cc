#include <catch.hpp>

#include <internal/values/simple_config_object.hpp>
#include <internal/values/simple_config_list.hpp>

#include "test_utils.hpp"

using namespace hocon;
using namespace std;
using namespace hocon::test_utils;

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
        shared_ptr<config_number> num = config_number::new_number(fake_origin(), int64_t(2), "2");
        REQUIRE(dynamic_pointer_cast<config_int>(num));
        REQUIRE_FALSE(dynamic_pointer_cast<config_long>(num));
    }

    SECTION("creates config_long from a large long") {
        int64_t definitely_a_long = numeric_limits<int64_t>::max();
        shared_ptr<config_number> num = config_number::new_number(fake_origin(),
            definitely_a_long, to_string(definitely_a_long));
        REQUIRE(dynamic_pointer_cast<config_long>(num));
        REQUIRE_FALSE(dynamic_pointer_cast<config_int>(num));
    }

    SECTION("creates config_int from small whole-number double") {
        shared_ptr<config_number> num = config_number::new_number(fake_origin(), 2.0, "2.0");
        REQUIRE(dynamic_pointer_cast<config_int>(num));
        REQUIRE_FALSE(dynamic_pointer_cast<config_double>(num));
    }

    SECTION("creates config_long from large whole-number double") {
        double big_double = numeric_limits<int>::max() + 1.0;
        shared_ptr<config_number> num = config_number::new_number(
                fake_origin(), big_double, to_string(big_double));
        REQUIRE(dynamic_pointer_cast<config_long>(num));
        REQUIRE_FALSE(dynamic_pointer_cast<config_double>(num));
    }

    SECTION("creates config_double for non-whole number") {
        shared_ptr<config_number> num = config_number::new_number(fake_origin(), 2.5, "2.5");
        REQUIRE(dynamic_pointer_cast<config_double>(num));
        REQUIRE_FALSE(dynamic_pointer_cast<config_int>(num));
    }
}

TEST_CASE("config object unwraps") {
    auto value1 = config_int::new_number(fake_origin(), int64_t(1), "1");
    auto value2 = config_int::new_number(fake_origin(), int64_t(2), "2");
    auto value3 = config_int::new_number(fake_origin(), int64_t(3), "3");
    unordered_map<string, shared_value> org {{"a", value1}, {"b", value2}, {"c", value3}};
    auto obj = make_shared<simple_config_object>(fake_origin(), org);
    unordered_map<string, unwrapped_value> map {{"a", 1}, {"b", 2}, {"c", 3}};
    unwrapped_value expected(map);
    bool test = expected == obj->unwrapped();
    REQUIRE(test);
}

TEST_CASE("config list unwraps") {
    auto value1 = config_int::new_number(fake_origin(), int64_t(1), "1");
    auto value2 = config_int::new_number(fake_origin(), int64_t(2), "2");
    auto value3 = config_int::new_number(fake_origin(), int64_t(3), "3");
    vector<shared_value> data { value1, value2, value3 };
    auto list = make_shared<simple_config_list>(fake_origin(), data);
    vector<unwrapped_value> v { 1,2,3 };
    unwrapped_value expected(v);
    bool test = expected == list->unwrapped();
    REQUIRE(test);
};
