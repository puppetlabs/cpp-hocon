#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/config_value_factory.hpp>
#include <internal/values/simple_config_object.hpp>

using namespace hocon;
using namespace std;

TEST_CASE("convert various types to config values", "[config_value_factory]") {
    SECTION("true should convert to config_boolean") {
        auto value = config_value_factory::from_any_ref(true, "");
        REQUIRE(dynamic_pointer_cast<const config_boolean>(value));
        REQUIRE(boost::get<bool>(value->unwrapped()) == true);
    }

    SECTION("false should convert to config_boolean") {
        auto value = config_value_factory::from_any_ref(false, "");
        REQUIRE(dynamic_pointer_cast<const config_boolean>(value));
        REQUIRE(boost::get<bool>(value->unwrapped()) == false);
    }

    SECTION("a null value should convert to config_null") {
        auto value = config_value_factory::from_any_ref(boost::blank(), "");
        REQUIRE(dynamic_pointer_cast<const config_null>(value));
        REQUIRE(boost::get<boost::blank>(value->unwrapped()) == boost::blank());
    }

    SECTION("string should convert to config_string") {
        auto value = config_value_factory::from_any_ref(string("test"), "");
        REQUIRE(dynamic_pointer_cast<const config_string>(value));
        REQUIRE(boost::get<string>(value->unwrapped()) == "test");
    }

    SECTION("int should convert to config_int") {
        auto value = config_value_factory::from_any_ref(2, "");
        REQUIRE(dynamic_pointer_cast<const config_int>(value));
        REQUIRE(boost::get<int>(value->unwrapped()) == 2);
    }

    SECTION("double should covert to config_double") {
        auto value = config_value_factory::from_any_ref(4.5, "");
        REQUIRE(dynamic_pointer_cast<const config_double>(value));
        REQUIRE(boost::get<double>(value->unwrapped()) == 4.5);
    }

    SECTION("long (int64_t) should convert to config_long") {
        auto value = config_value_factory::from_any_ref(int64_t(19), "");
        REQUIRE(dynamic_pointer_cast<const config_long>(value));
        REQUIRE(boost::get<int64_t>(value->unwrapped()) == 19);
    }

    SECTION("map should convert to simple_config_object") {
        unordered_map<string, unwrapped_value> map {{ "a", 1 }, { "b", "string" }, { "c", false }};
        auto value = config_value_factory::from_any_ref(map, "");
        REQUIRE(dynamic_pointer_cast<const simple_config_object>(value));
        auto unwrapped = boost::get<unordered_map<string, unwrapped_value>>(value->unwrapped());
        REQUIRE(unwrapped == map);
    }
}
