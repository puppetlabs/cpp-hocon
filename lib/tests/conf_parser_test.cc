#include <catch.hpp>
#include "test_utils.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <hocon/config.hpp>
#include <internal/values/simple_config_object.hpp>

using namespace std;
using namespace hocon;

static size_t get_size(shared_object obj) {
    auto simple_obj = dynamic_pointer_cast<const simple_config_object>(obj);
    REQUIRE(simple_obj);
    return simple_obj->size();
}

TEST_CASE("duplicate key last wins") {
    auto obj = parse_config(R"({ "a" : 10, "a" : 11 } )");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(11 == obj->get_int("a"));
}

TEST_CASE("duplicate key objects merged") {
    auto obj = parse_config(R"({ "a" : { "x" : 1, "y" : 2 }, "a" : { "x" : 42, "z" : 100 } })");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(3u == get_size(obj->get_object("a")));
    REQUIRE(42 == obj->get_int("a.x"));
    REQUIRE(2 == obj->get_int("a.y"));
    REQUIRE(100 == obj->get_int("a.z"));
}

TEST_CASE("duplicate key objects merged recursively") {
    auto obj = parse_config(R"({ "a" : { "b" : { "x" : 1, "y" : 2 } }, "a" : { "b" : { "x" : 42, "z" : 100 } } })");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(1u == get_size(obj->get_object("a")));
    REQUIRE(3u == get_size(obj->get_object("a.b")));
    REQUIRE(42 == obj->get_int("a.b.x"));
    REQUIRE(2 == obj->get_int("a.b.y"));
    REQUIRE(100 == obj->get_int("a.b.z"));
}

TEST_CASE("duplicate key objects merged recursively deeper") {
    auto obj = parse_config(R"({ "a" : { "b" : { "c" : { "x" : 1, "y" : 2 } } }, "a" : { "b" : { "c" : { "x" : 42, "z" : 100 } } } })");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(1u == get_size(obj->get_object("a")));
    REQUIRE(1u == get_size(obj->get_object("a.b")));
    REQUIRE(3u == get_size(obj->get_object("a.b.c")));
    REQUIRE(42 == obj->get_int("a.b.c.x"));
    REQUIRE(2 == obj->get_int("a.b.c.y"));
    REQUIRE(100 == obj->get_int("a.b.c.z"));
}

TEST_CASE("duplicate key object null object") {
    auto obj = parse_config(R"({ a : { b : 1 }, a : null, a : { c : 2 } })");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(1u == get_size(obj->get_object(("a"))));
    REQUIRE(2 == obj->get_int("a.c"));
}

TEST_CASE("duplicate key object number object") {
    auto obj = parse_config(R"({ a : { b : 1 }, a : 42, a : { c : 2 } })");
    REQUIRE(1u == get_size(obj->root()));
    REQUIRE(1u == get_size(obj->get_object("a")));
    REQUIRE(2 == obj->get_int("a.c"));
}
