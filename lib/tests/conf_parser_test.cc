#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/config.hpp>
#include <internal/values/simple_config_object.hpp>

using namespace std;
using namespace hocon;

TEST_CASE("duplicate key last wins") {
    auto obj = parse_config("{ \"a\" : 10, \"a\" : 11 } ");
    auto simple_obj = dynamic_pointer_cast<const simple_config_object>(obj->root());
    REQUIRE(simple_obj);
    REQUIRE(1u == simple_obj->size());
    REQUIRE(11 == obj->get_int("a"));
}