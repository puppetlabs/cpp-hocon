#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/config.hpp>
#include <internal/values/simple_config_object.hpp>

#include <internal/resolve_result.hpp>
#include <hocon/config_resolve_options.hpp>
#include <internal/resolve_context.hpp>
#include <hocon/config_exception.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <leatherman/util/environment.hpp>
#include <unordered_map>

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

static shared_object subst_chain_object() {
    static auto const resolved = parse_object(R"(
{
    "foo" : ${bar},
    "bar" : ${a.b.c},
    "a" : { "b" : { "c" : 57 } }
}
)");

    return resolved;
}

static shared_object subst_complex_object() {
    static auto const resolved = parse_object(R"(
{
    "foo" : ${bar},
    "bar" : ${a.b.c},
    "a" : { "b" : { "c" : 57, "d" : ${foo}, "e" : { "f" : ${foo} } } },
    "objA" : ${a},
    "objB" : ${a.b},
    "objE" : ${a.b.e},
    "foo.bar" : 37,
    "arr" : [ ${foo}, ${a.b.c}, ${"foo.bar"}, ${objB.d}, ${objA.b.e.f}, ${objE.f} ],
    "ptrToArr" : ${arr},
    "x" : { "y" : { "ptrToPtrToArr" : ${ptrToArr} } }
}
)");

    return resolved;
}

static shared_object subst_env_var_object() {
    static auto const resolved = parse_object(R"(
{
    "home" : ${?HOME},
    "pwd" : ${?PWD},
    "shell" : ${?SHELL},
    "lang" : ${?LANG},
    "path" : ${?PATH},
    "not_here" : ${?NOT_HERE}
}
)");

    return resolved;
}

static shared_config resolve_without_fallbacks (shared_object v) {
    auto options = config_resolve_options(false);
    return dynamic_pointer_cast<const config_object>(resolve_context::resolve(v, v, options))->to_config();
}

static shared_value resolve_without_fallbacks (shared_value s, shared_object root) {
    auto options = config_resolve_options(false);
    return resolve_context::resolve(s, root, options);
}

static shared_config resolve (shared_object v) {
    auto options = config_resolve_options(false);
    return dynamic_pointer_cast<const config_object>(resolve_context::resolve(v, v, options))->to_config();
}

TEST_CASE("basic resolutions") {
    SECTION("resolve trivial key") {
        auto s = subst("foo");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*int_value(42) == *v);
    }

    // the "resolve int" test is exactly the same as
    // "resolve trivial path" so I did not repeat it
    SECTION("resolve trivial path") {
        auto s = subst("bar.int");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*int_value(43) == *v);
    }

    SECTION("resolve bool") {
        auto s = subst("bar.bool");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*bool_value(true) == *v);
    }

    SECTION("resolve null") {
        auto s = subst("bar.null");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*null_value() == *v);
    }

    SECTION("resolve string") {
        auto s = subst("bar.string");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("hello") == *v);
    }

    SECTION("resolve double") {
        auto s = subst("bar.double");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*double_value(3.14) == *v);
    }

    SECTION("resolve missing throws") {
        bool thrown = false;
        auto s = subst("bar.missing");
        try {
            auto v = resolve_without_fallbacks(s, simple_object());
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_NOT_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }
}

TEST_CASE("basic resolutions in strings") {
    SECTION("resolve int in string") {
        auto s = subst_in_string("bar.int");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("start<43>end") == *v);
    }

    SECTION("resolve null in string") {
        auto s = subst_in_string("bar.null");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("start<null>end") == *v);

        // when null is NOT a subst, it should also not become empty
        auto o = parse_config("{ \"a\" : null foo bar }");
        REQUIRE("null foo bar" == o->get_string("a"));
    }

    SECTION("resolve bool in string") {
        auto s = subst_in_string("bar.bool");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("start<true>end") == *v);
    }

    SECTION("resolve string in string") {
        auto s = subst_in_string("bar.string");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("start<hello>end") == *v);
    }

    SECTION("resolve double in string") {
        auto s = subst_in_string("bar.double");
        auto v = resolve_without_fallbacks(s, simple_object());
        REQUIRE(*string_value("start<3.14>end") == *v);
    }
}

TEST_CASE("chain substitution") {
    auto s = subst("foo");
    auto v = resolve_without_fallbacks(s, subst_chain_object());
    REQUIRE(*int_value(57) == *v);
}

TEST_CASE("substitutions look forward") {
    auto obj = parse_object("a=1,b=${a},a=2");
    auto resolved = resolve(obj);
    REQUIRE(2u == resolved->get_int("b"));
}

TEST_CASE("cycles") {
    SECTION("throw on incredibly trivial cycle") {
        bool thrown = false;
        auto s = subst("a");
        try {
            auto v = resolve_without_fallbacks(s, parse_object("a: ${a}"));
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }

    SECTION("throw on cycles") {
        bool thrown = false;
        auto s = subst("foo");
        try {
            auto v = resolve_without_fallbacks(s, subst_cycle_object());
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }

    SECTION("throw on optional reference to non optional cycle") {
        // we look up ${?foo}, but the cycle has hard
        // non-optional links in it so still has to throw.
        bool thrown = false;
        auto s = subst("foo", true);
        try {
            auto v = resolve_without_fallbacks(s, subst_cycle_object());
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }

    SECTION("optional link cycles act like undefined") {
        auto s = subst("foo", true);
        auto v = resolve_without_fallbacks(s, subst_cycle_object_optional_link());
        REQUIRE(v == nullptr);
    }

    SECTION("throw on two key cycle") {
        bool thrown = false;
        auto obj = parse_object("a:${b},b:${a}");
        try {
            resolve(obj);
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }

        REQUIRE(thrown);
    }

    SECTION("throw on four key cycle") {
        bool thrown = false;
        auto obj = parse_object("a:${b},b:${c},c:${d},d:${a}");
        try {
            resolve(obj);
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }

        REQUIRE(thrown);
    }
}

TEST_CASE("resolve object") {
    auto resolved = resolve_without_fallbacks(subst_chain_object());
    REQUIRE(57u == resolved->get_int("foo"));
    REQUIRE(57u == resolved->get_int("bar"));
    REQUIRE(57u == resolved->get_int("a.b.c"));
}

TEST_CASE("ignore hidden undefined subst") {
    // if a substitution is overridden then it shouldn't matter that it's undefined
    auto obj = parse_object("a=${nonexistent},a=42");
    auto resolved = resolve(obj);
    REQUIRE(42u == resolved->get_int("a"));
}

TEST_CASE("ignore hidden circular subst") {
    // if a substitution is overridden then it shouldn't matter that it's circular
    auto obj = parse_object("a=${a},a=42");
    auto resolved = resolve(obj);
    REQUIRE(42u == resolved->get_int("a"));
}

/* TODO: add tests after createValueUnderPath is implemented: HC-66
 * avoidDelayedMergeObjectResolveProblem1
 * avoidDelayedMergeObjectResolveProblem2
 * avoidDelayedMergeObjectResolveProblem3
 * avoidDelayedMergeObjectResolveProblem4
 * avoidDelayedMergeObjectResolveProblem5
 * avoidDelayedMergeObjectResolveProblem6
 * fetchKnownValueFromDelayedMergeObject
 * delayedMergeObjectNeedsFullResolve
 * failToFetchFromDelayedMergeObjectNeedsFullResolve
 * resolveDelayedMergeObjectEmbrace
 * resolvePlainObectEmbrace
 */

// TODO: this test legitimately fails: HC-72
TEST_CASE("pending HC-72: use relative to same file when relativized", "[!shouldfail]") {
    auto child = parse_object("foo=in child,bar=${foo}");
    auto values = unordered_map<string, shared_value> {};

    values.insert(pair<string, shared_value>("a", child->relativized("a")));
    // this "foo" should NOT be used
    values.insert(pair<string, shared_value>("foo", string_value("in parent")));

    auto resolved = resolve(make_shared<simple_config_object>(fake_origin(), values));
    REQUIRE("in child" == resolved->get_string("a.bar"));
}

TEST_CASE("use relative to root when relativized") {
    // here, "foo" is not defined in the child
    auto child = parse_object("bar=${foo}");
    auto values = unordered_map<string, shared_value> {};

    values.insert(pair<string, shared_value>("a", child->relativized("a")));
    // so this "foo" SHOULD be used
    values.insert(pair<string, shared_value>("foo", string_value("in parent")));

    auto resolved = resolve(make_shared<simple_config_object>(fake_origin(), values));
    REQUIRE("in parent" == resolved->get_string("a.bar"));
}

// TODO: this test legitimately fails: HC-73
TEST_CASE("pending HC-73: complex resolve", "[!shouldfail]") {
    auto resolved = resolve_without_fallbacks(subst_complex_object());

    REQUIRE(57u == resolved->get_int("foo"));
    REQUIRE(57u == resolved->get_int("bar"));
    REQUIRE(57u == resolved->get_int("a.b.c"));
    REQUIRE(57u == resolved->get_int("a.b.d"));
    REQUIRE(57u == resolved->get_int("objB.d"));
}


// TODO: env variable fallback legitimately fails: HC-74
TEST_CASE("pending HC-74: fallback to env", "[!shouldfail]") {
    auto resolved = resolve(subst_env_var_object());
    int existed = 0;
    auto list = dynamic_pointer_cast<const simple_config_object>(resolved->root());

    for (auto&& k : list->key_set()) {
        string e;
        auto env_var_exists = leatherman::util::environment::get(k, e);
        if (env_var_exists) {
            existed += 1;
            REQUIRE(e == resolved->get_string(k));
        } else {
            REQUIRE(nullptr == resolved->root()->get(k));
        }
    }
    if (existed == 0) {
        throw config_exception("None of the env vars we tried to use for testing were set");
    }
}

/* TODO: add tests after environment lookup bug is fixed: HC-74
 * noFallbackToEnvIfValuesAreNull
 * fallbackToEnvWhenRelativized
*/

TEST_CASE("throw when env not found") {
    bool thrown = false;
    auto obj = parse_object("{ a : ${NOT_HERE} }");

    try {
        resolve(obj);
    } catch (const hocon::config_exception& e) {
        thrown = true;
    }
    REQUIRE(thrown);
}

TEST_CASE("optional override not provided") {
    auto obj = parse_object("{ a: 42, a : ${?NOT_HERE} }");
    auto resolved = resolve(obj);
    REQUIRE(42u == resolved->get_int("a"));
}

TEST_CASE("optional override provided") {
    auto obj = parse_object("{ HERE : 43, a: 42, a : ${?HERE} }");
    auto resolved = resolve(obj);
    REQUIRE(43u == resolved->get_int("a"));
}

TEST_CASE("optional override of object not provided") {
    auto obj = parse_object("{ a: { b : 42 }, a : ${?NOT_HERE} }");
    auto resolved = resolve(obj);
    REQUIRE(42u == resolved->get_int("a.b"));
}

TEST_CASE("optional override of object provided") {
    auto obj = parse_object("{ HERE : 43, a: { b : 42 }, a : ${?HERE} }");
    auto resolved = resolve(obj);
    REQUIRE(43u == resolved->get_int("a"));
    REQUIRE_FALSE(resolved->has_path("a.b"));
}

// TODO: Uncomment this test and write optionalUsedInArray when config::get_*_list are implemented: HC-75
TEST_CASE("pending HC-75: optional vanishes from array", "[!shouldfail]") {
    auto obj = parse_object("{ a : [ 1, 2, 3, ${?NOT_HERE} ] }");
    auto resolved = resolve(obj);
    auto list = vector<int> {1,2,3};
    REQUIRE(list == resolved->get_int_list("a"));
}

TEST_CASE("subst self references", "[!shouldfail]") {
    SECTION("subst self reference") {
        auto obj = parse_object("a=1, a=${a}");
        auto resolved = resolve(obj);
        REQUIRE(1u == resolved->get_int("a"));
    }

    SECTION("subst self reference undefined") {
        bool thrown = false;
        auto obj = parse_object("a=${a}");

        try {
            resolve(obj);
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }

    SECTION("subst self reference optional") {
        auto obj = parse_object("a=${?a}");
        auto resolved = resolve(obj);
        REQUIRE(0u == resolved->root()->size());
    }

    // TODO: these two should pass when createValuePath is implemented: HC-66
    SECTION("pending HC-66: subst self reference along path", "[!shouldfail]") {
        auto obj = parse_object("a.b=1, a.b=${a.b}");
        auto resolved = resolve(obj);
        REQUIRE(1u == resolved->get_int("a.b"));
    }

    SECTION("pending HC-66: subst self reference along longer path", "[!shouldfail]") {
        auto obj = parse_object("a.b.c=1, a.b.c=${a.b.c}");
        auto resolved = resolve(obj);
        REQUIRE(1u == resolved->get_int("a.b.c"));
    }

    /* TODO: also write:
     * substSelfReferenceAlongPathMoreComplex
     * substSelfReferenceObjectAlongPath
     * substSelfReferenceAlongAPath
     * substSelfReferenceAlongAPathInsideObject
    */

    SECTION("subst self reference object") {
        auto obj = parse_object("a={b=5}, a=${a}");
        auto resolved = resolve(obj);
        REQUIRE(5u == resolved->get_int("a.b"));
    }

    SECTION("subst self reference in concat") {
        auto obj = parse_object("a=1, a=${a}foo");
        auto resolved = resolve(obj);
        REQUIRE("1foo" == resolved->get_string("a"));
    }

    SECTION("subst optional self reference in concat") {
        auto obj = parse_object("a=${?a}foo");
        auto resolved = resolve(obj);
        REQUIRE("foo" == resolved->get_string("a"));
    }

    SECTION("subst optional indirect self reference in concat") {
        auto obj = parse_object("a=${?b}foo,b=${?a}");
        auto resolved = resolve(obj);
        REQUIRE("foo" == resolved->get_string("a"));
    }

    SECTION("subst two optional self references in concat with prior value") {
        auto obj = parse_object("a=1,a=${?a}foo${?a}");
        auto resolved = resolve(obj);
        REQUIRE("1foo1" == resolved->get_string("a"));
    }

    SECTION("subst self reference middle of stack") {
        auto obj = parse_object("a=1, a=${a}, a=2");
        auto resolved = resolve(obj);
        // the substitution would be 1, but then 2 overrides
        REQUIRE(2u == resolved->get_int("a"));
    }

    // TODO: this test legitimately fails: HC-76
    SECTION("pending HC-76: subst self reference object middle of stack", "[!shouldfail]") {
        auto obj = parse_object("a={b=5}, a=${a}, a={c=6}");
        auto resolved = resolve(obj);
        REQUIRE(5u == resolved->get_int("a.b"));
        REQUIRE(6u == resolved->get_int("a.c"));
    }

    SECTION("subst optional self reference middle of stack") {
        auto obj = parse_object("a=1, a=${?a}, a=2");
        auto resolved = resolve(obj);
        // the substitution would be 1, but then 2 overrides
        REQUIRE(2u == resolved->get_int("a"));
    }

    SECTION("subst self reference bottom of stack") {
        // self-reference should just be ignored since it's
        // overridden
        auto obj = parse_object("a=${a}, a=1, a=2");
        auto resolved = resolve(obj);
        // the substitution would be 1, but then 2 overrides
        REQUIRE(2u == resolved->get_int("a"));
    }

    SECTION("subst optional self reference bottom of stack") {
        auto obj = parse_object("a=${?a}, a=1, a=2");
        auto resolved = resolve(obj);
        // the substitution would be 1, but then 2 overrides
        REQUIRE(2u == resolved->get_int("a"));
    }

    SECTION("subst self reference top of stack") {
        auto obj = parse_object("a=1, a=2, a=${a}");
        auto resolved = resolve(obj);
        REQUIRE(2u == resolved->get_int("a"));
    }

    SECTION("subst optional self reference top of stack") {
        auto obj = parse_object("a=1, a=2, a=${?a}");
        auto resolved = resolve(obj);
        REQUIRE(2u == resolved->get_int("a"));
    }

    SECTION("subst in child field not a self reference 1") {
        // here, ${bar.foo} is not a self reference because
        // it's the value of a child field of bar, not bar
        // itself; so we use bar's current value, rather than
        // looking back in the merge stack
        auto obj = parse_object(R"(
             bar : { foo : 42,
                     baz : ${bar.foo}
             }
        )");

        auto resolved = resolve(obj);
        REQUIRE(42u == resolved->get_int("bar.baz"));
        REQUIRE(42u == resolved->get_int("bar.foo"));
    }

    SECTION("subst in child field not a self reference 2") {
        // checking that having bar.foo later in the stack
        // doesn't break the behavior
        auto obj = parse_object(R"(
             bar : { foo : 42,
                     baz : ${bar.foo}
             }
             bar : { foo : 43 }
        )");

        auto resolved = resolve(obj);
        REQUIRE(43u == resolved->get_int("bar.baz"));
        REQUIRE(43u == resolved->get_int("bar.foo"));
    }

    SECTION("subst in child field not a self reference 3") {
        // checking that having bar.foo earlier in the merge
        // stack doesn't break the behavior.
        auto obj = parse_object(R"(
             bar : { foo : 43 }
             bar : { foo : 42,
                     baz : ${bar.foo}
             }
        )");

        auto resolved = resolve(obj);
        REQUIRE(42u == resolved->get_int("bar.baz"));
        REQUIRE(42u == resolved->get_int("bar.foo"));
    }

    // TODO: should pass when createValuePath is implemented: HC-66
    SECTION("pending HC-66: mutually referring not a self reference", "[!shouldfail]") {
        auto obj = parse_object(R"(
            // bar.a should end up as 4
            bar : { a : ${foo.d}, b : 1 }
            bar.b = 3
            // foo.c should end up as 3
            foo : { c : ${bar.b}, d : 2 }
            foo.d = 4
        )");
        auto resolved = resolve(obj);
        REQUIRE(4u == resolved->get_int("bar.a"));
        REQUIRE(3u == resolved->get_int("foo.c"));
    }

    // TODO: these tests legitimately fail: HC-77
    SECTION("pending HC-77: subst self reference multiple times", "[!shouldfail]") {
        auto obj = parse_object("a=1,a=${a},a=${a},a=${a}");
        auto resolved = resolve(obj);
        REQUIRE(1u == resolved->get_int("a"));
    }

    SECTION("pending HC-77: subst self reference in concat multiple times", "[!shouldfail]") {
        auto obj = parse_object("a=1,a=${a}x,a=${a}y,a=${a}z");
        auto resolved = resolve(obj);
        REQUIRE("1xyz" == resolved->get_string("a"));
    }

    SECTION("subst self reference in array") {
        // never "look back" from "inside" an array
        bool thrown = false;
        auto obj = parse_object("a=1,a=[${a}, 2]");
        try {
            resolve(obj);
        } catch (const hocon::config_exception &e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "cycle");
        }
        REQUIRE(thrown);
    }
}