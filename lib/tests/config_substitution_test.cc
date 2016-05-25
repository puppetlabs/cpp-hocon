#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/config.hpp>
#include <internal/values/simple_config_object.hpp>

#include <internal/resolve_result.hpp>
#include <hocon/config_resolve_options.hpp>
#include <internal/resolve_context.hpp>
#include <hocon/config_exception.hpp>

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

static shared_object simple_object() {
    static auto const resolved = parse_object(R"(
{
    "foo" : 42,
    "bar" : {
        "int" : 43,
        "bool" : true,
        "null" : null,
        "string" : "hello",
        "double" : 3.14
    }
}
)");

    return resolved;
}

static shared_object subst_cycle_object() {
    static auto const resolved = parse_object(R"(
{
    "foo" : ${bar},
    "bar" : ${a.b.c},
    "a" : {
      "b" : {
        "c" : ${foo}
      }
    }
}
)");

    return resolved;
}

static shared_object subst_cycle_object_optional_link() {
    static auto const resolved = parse_object(R"(
{
    "foo" : ${?bar},
    "bar" : ${?a.b.c},
    "a" : {
      "b" : {
        "c" : ${?foo}
      }
    }
}
)");

    return resolved;
}

static shared_value resolve_without_fallbacks (shared_value s, shared_object root) {
    auto options = config_resolve_options(false);
    return resolve_context::resolve(s, root, options);
}

static shared_value resolve (shared_object v) {
    auto options = config_resolve_options(false);
    return resolve_context::resolve(v, v, options);
}

TEST_CASE("resolve trivial key") {
    auto s = subst("foo");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*int_value(42) == *v);
}

// the "resolve int" test is exactly the same as
// "resolve trivial path" so I did not repeat it
TEST_CASE("resolve trivial path") {
    auto s = subst("bar.int");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*int_value(43) == *v);
}

TEST_CASE("resolve bool") {
    auto s = subst("bar.bool");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*bool_value(true) == *v);
}

TEST_CASE("resolve null") {
    auto s = subst("bar.null");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*null_value() == *v);
}

TEST_CASE("resolve string") {
    auto s = subst("bar.string");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*string_value("hello") == *v);
}

TEST_CASE("resolve double") {
    auto s = subst("bar.double");
    auto v = resolve_without_fallbacks(s, simple_object());
    REQUIRE(*double_value(3.14) == *v);
}

TEST_CASE("throw on incredibly trivial cycle") {
    bool thrown = false;
    auto s = subst("a");
    try {
        auto v = resolve_without_fallbacks(s, parse_object("a: ${a}"));
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "cycle");
    }
    REQUIRE(thrown);
}

TEST_CASE("throw on cycles") {
    bool thrown = false;
    auto s = subst("foo");
    try {
        auto v = resolve_without_fallbacks(s, subst_cycle_object());
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "cycle");
    }
    REQUIRE(thrown);
}

TEST_CASE("throw on optional reference to non optional cycle") {
    // we look up ${?foo}, but the cycle has hard
    // non-optional links in it so still has to throw.
    bool thrown = false;
    auto s = subst("foo", true);
    try {
        auto v = resolve_without_fallbacks(s, subst_cycle_object());
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "cycle");
    }
    REQUIRE(thrown);
}

TEST_CASE("optional link cycles act like undefined") {
    auto s = subst("foo", true);
    auto v = resolve_without_fallbacks(s, subst_cycle_object_optional_link());
    REQUIRE(v == nullptr);
}

TEST_CASE("throw on two key cycle") {
    bool thrown = false;
    auto obj = parse_object("a:${b},b:${a}");
    try {
        resolve(obj);
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "cycle");
    }

    REQUIRE(thrown);
}

TEST_CASE("throw on four key cycle") {
    bool thrown = false;
    auto obj = parse_object("a:${b},b:${c},c:${d},d:${a}");
    try {
        resolve(obj);
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "cycle");
    }

    REQUIRE(thrown);
}