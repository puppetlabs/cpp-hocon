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

static shared_value resolve_without_fallbacks (shared_value s, shared_object root) {
    auto options = config_resolve_options(false);
    return resolve_context::resolve(s, root, options);
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

