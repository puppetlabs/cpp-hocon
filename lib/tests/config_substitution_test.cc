#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/config.hpp>
#include <internal/values/simple_config_object.hpp>

#include <internal/resolve_result.hpp>
#include <hocon/config_resolve_options.hpp>
#include <internal/resolve_context.hpp>
#include <hocon/config_exception.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <internal/values/config_delayed_merge.hpp>
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

// TODO: the following tests dealing with delayed merge objects do not pass, see HC-78

TEST_CASE("(pending HC-78) avoid delayed merge object problem 1", "[!shouldfail]") {
    auto problem = parse_object(R"(
    defaults {
            a = 1
            b = 2
    }
    // make item1 into a ConfigDelayedMergeObject
    item1 = ${defaults}
    // note that we'll resolve to a non-object value
    // so item1.b will ignoreFallbacks and not depend on
    // ${defaults}
    item1.b = 3
    // be sure we can resolve a substitution to a value in
    // a delayed-merge object.
    item2.b = ${item1.b}
      )");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(problem->attempt_peek_with_partial_resolve("item1")));

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(3 == resolved->get_int("item1.b"));
    REQUIRE(3 == resolved->get_int("item2.b"));
}

TEST_CASE("(pending HC-78) avoid delayed merge object resolve problem 2", "[!shouldfail]") {
    auto problem = parse_object(R"(
defaults {
    a = 1
    b = 2
  }
  // make item1 into a ConfigDelayedMergeObject
  item1 = ${defaults}
  // note that we'll resolve to an object value
  // so item1.b will depend on also looking up ${defaults}
  item1.b = { c : 43 }
  // be sure we can resolve a substitution to a value in
  // a delayed-merge object.
  item2.b = ${item1.b}
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(problem->attempt_peek_with_partial_resolve("item1")));

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(parse_object("{ c : 43 }") == resolved->get_object("item1.b"));
    REQUIRE(43 == resolved->get_int("item1.b.c"));
    REQUIRE(43 == resolved->get_int("item2.b.c"));

}

TEST_CASE("(pending HC-78) avoid delayed merge object resolve problem 3", "[!shouldfail]") {
    auto problem = parse_object(R"(
item1.b.c = 100
  defaults {
    // we depend on item1.b.c
    a = ${item1.b.c}
    b = 2
  }
  // make item1 into a ConfigDelayedMergeObject
  item1 = ${defaults}
  // the ${item1.b.c} above in ${defaults} should ignore
  // this because it only looks back
  item1.b = { c : 43 }
  // be sure we can resolve a substitution to a value in
  // a delayed-merge object.
  item2.b = ${item1.b}
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(problem->attempt_peek_with_partial_resolve("item1")));

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(parse_object("{ c : 43 }") == resolved->get_object("item1.b"));
    REQUIRE(43 == resolved->get_int("item1.b.c"));
    REQUIRE(43 == resolved->get_int("item2.b.c"));
    REQUIRE(100 == resolved->get_int("defaults.a"));
}

TEST_CASE("(pending HC-78) avoid delayed merge object resolve problem 4", "[!shouldfail]") {
    auto problem = parse_object(R"(
defaults {
    a = 1
    b = 2
  }

  item1.b = 7
  // make item1 into a ConfigDelayedMerge
  item1 = ${defaults}
  // be sure we can resolve a substitution to a value in
  // a delayed-merge object.
  item2.b = ${item1.b}
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge>(problem->attempt_peek_with_partial_resolve("item1")));

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(2 == resolved->get_int("item1.b"));
    REQUIRE(2 == resolved->get_int("item2.b"));
}

TEST_CASE("(pending HC-78) avoid delayed merge object resolve problem 5", "[!shouldfail]") {
    auto problem = parse_object(R"(
defaults {
    a = ${item1.b} // tricky cycle - we won't see ${defaults}
                   // as we resolve this
    b = 2
  }

  item1.b = 7
  // make item1 into a ConfigDelayedMerge
  item1 = ${defaults}
  // be sure we can resolve a substitution to a value in
  // a delayed-merge object.
  item2.b = ${item1.b}
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge>(problem->attempt_peek_with_partial_resolve("item1")));

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(2 == resolved->get_int("item1.b"));
    REQUIRE(2 == resolved->get_int("item2.b"));
    REQUIRE(7 == resolved->get_int("defauls.a"));
}

TEST_CASE("(pending HC-78) avoid delayed merge object resolve problem 6", "[!shouldfail]") {
    auto problem = parse_object(R"(
z = 15
  defaults-defaults-defaults {
    m = ${z}
    n.o.p = ${z}
  }
  defaults-defaults {
    x = 10
    y = 11
    asdf = ${z}
  }
  defaults {
    a = 1
    b = 2
  }
  defaults-alias = ${defaults}
  // make item1 into a ConfigDelayedMergeObject several layers deep
  // that will NOT become resolved just because we resolve one path
  // through it.
  item1 = 345
  item1 = ${?NONEXISTENT}
  item1 = ${defaults-defaults-defaults}
  item1 = {}
  item1 = ${defaults-defaults}
  item1 = ${defaults-alias}
  item1 = ${defaults}
  item1.b = { c : 43 }
  item1.xyz = 101
  // be sure we can resolve a substitution to a value in
  // a delayed-merge object.
  item2.b = ${item1.b}
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(problem->attempt_peek_with_partial_resolve("item1")));

    unwrapped_value expected(10);
    bool test = expected == problem->to_config()->get_object("item1")->attempt_peek_with_partial_resolve("xyz")->unwrapped();
    REQUIRE(test);

    auto resolved = resolve_without_fallbacks(problem);
    REQUIRE(parse_object("{ c : 43 }") == resolved->get_object("item1.b"));
    REQUIRE(43 == resolved->get_int("item1.b.c"));
    REQUIRE(43 == resolved->get_int("item2.b.c"));
    REQUIRE(15 == resolved->get_int("item1.n.o.p"));
}

TEST_CASE("Fetch known value from delayed merge object") {
    auto obj = parse_object(R"(
defaults {
    a = 1
    b = 2
  }
  // make item1 into a ConfigDelayedMergeObject
  item1 = ${defaults}
  // note that we'll resolve to a non-object value
  // so item1.b will ignoreFallbacks and not depend on
  // ${defaults}
  item1.b = 3
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(obj->attempt_peek_with_partial_resolve("item1")));
    REQUIRE(3 == obj->to_config()->get_config("item1")->get_int("b"));
}

TEST_CASE("Fail to fetch from delayed merge object needs full resolve") {
    auto obj = parse_object(R"(
  defaults {
    a = 1
    b = { c : 31 }
  }
  item1 = ${defaults}
  // because b is an object, fetching it requires resolving ${defaults} above
  // to see if there are more keys to merge with b.
  item1.b = { c : 41 })");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(obj->attempt_peek_with_partial_resolve("item1")));
    REQUIRE_THROWS(obj->to_config()->get_object("item1.b"));
}

TEST_CASE("(pending HC-78) resolve delayed merge object embrace", "[!shouldfail]") {
    auto obj = parse_object(R"(
  defaults {
    a = 1
    b = 2
  }

  item1 = ${defaults}
  // item1.c refers to a field in item2 that refers to item1
  item1.c = ${item2.d}
  // item1.x refers to a field in item2 that doesn't go back to item1
  item1.x = ${item2.y}

  item2 = ${defaults}
  // item2.d refers to a field in item1
  item2.d = ${item1.a}
  item2.y = 15
)");

    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(obj->attempt_peek_with_partial_resolve("item1")));
    REQUIRE(dynamic_pointer_cast<const config_delayed_merge_object>(obj->attempt_peek_with_partial_resolve("item2")));

    auto resolved = obj->to_config()->resolve();
    REQUIRE(1 == resolved->get_int("item1.c"));
    REQUIRE(1 == resolved->get_int("item2.d"));
    REQUIRE(15 == resolved->get_int("item1.x"));
}

TEST_CASE("resolve plain object embrace") {
    auto obj = parse_object(R"(
  item1.a = 10
  item1.b = ${item2.d}
  item2.c = 12
  item2.d = 14
  item2.e = ${item1.a}
  item2.f = ${item1.b}   // item1.b goes back to item2
  item2.g = ${item2.f}   // goes back to ourselves
)");

    REQUIRE(dynamic_pointer_cast<const simple_config_object>(obj->attempt_peek_with_partial_resolve("item1")));
    REQUIRE(dynamic_pointer_cast<const simple_config_object>(obj->attempt_peek_with_partial_resolve("item2")));

    auto resolved = obj->to_config()->resolve();
    REQUIRE(14 == resolved->get_int("item1.b"));
    REQUIRE(10 == resolved->get_int("item2.e"));
    REQUIRE(14 == resolved->get_int("item2.f"));
    REQUIRE(14 == resolved->get_int("item2.g"));
}

// TODO: this test legitimately fails: HC-72
TEST_CASE("pending HC-72: use relative to same file when relativized (pending)", "[!shouldfail]") {
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
TEST_CASE("pending HC-73: complex resolve (pending)", "[!shouldfail]") {
    auto resolved = resolve_without_fallbacks(subst_complex_object());

    REQUIRE(57u == resolved->get_int("foo"));
    REQUIRE(57u == resolved->get_int("bar"));
    REQUIRE(57u == resolved->get_int("a.b.c"));
    REQUIRE(57u == resolved->get_int("a.b.d"));
    REQUIRE(57u == resolved->get_int("objB.d"));
}


// TODO: env variable fallback legitimately fails: HC-74
TEST_CASE("pending HC-74: fallback to env (pending)", "[!shouldfail]") {
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

TEST_CASE("optional vanishes from array") {
    auto obj = parse_object("{ a : [ 1, 2, 3, ${?NOT_HERE} ] }");
    auto resolved = resolve(obj);
    auto list = vector<int> {1,2,3};
    REQUIRE(list == resolved->get_int_list("a"));
}


TEST_CASE("subst self references") {
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

    SECTION("subst self reference along path") {
        auto obj = parse_object("a.b=1, a.b=${a.b}");
        auto resolved = resolve(obj);
        REQUIRE(1u == resolved->get_int("a.b"));
    }

    SECTION("subst self reference along longer path") {
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

    SECTION("mutually referring not a self reference") {
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

TEST_CASE("subst self references (pending)", "[!shouldfail]") {
    // TODO: this test legitimately fails: HC-76
    SECTION("pending HC-76: subst self reference object middle of stack", "[!shouldfail]") {
        auto obj = parse_object("a={b=5}, a=${a}, a={c=6}");
        auto resolved = resolve(obj);
        REQUIRE(5u == resolved->get_int("a.b"));
        REQUIRE(6u == resolved->get_int("a.c"));
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
}
