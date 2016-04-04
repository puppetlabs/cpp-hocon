#include <catch.hpp>
#include "test_utils.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <hocon/config.hpp>
#include <hocon/config_exception.hpp>
#include <hocon/config_parse_options.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/parseable.hpp>
#include <internal/resolve_context.hpp>

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

static shared_value parse_without_resolving(string s) {
    auto options = config_parse_options()
        .set_origin_description(make_shared<string>("test conf string"))
        .set_syntax(config_syntax::CONF);
    return parseable::new_string(move(s), move(options)).parse_value();
}

static shared_value parse(string s) {
    auto tree = parse_without_resolving(move(s));
    if (auto obj = dynamic_pointer_cast<const config_object>(tree)) {
        return resolve_context::resolve(tree, obj, config_resolve_options(false));
    }
    return tree;
}

TEST_CASE("invalid conf throws") {
    for (auto const& invalid : whitespace_variations(invalid_conf(), false)) {
        CAPTURE(invalid.test);
        REQUIRE_THROWS_AS(parse(invalid.test), config_exception);
    }
}

TEST_CASE("valid conf works") {
    // all we're checking here unfortunately is that it doesn't throw.
    // for a more thorough check, use the EquivalentsTest stuff.
    for (auto const& valid : whitespace_variations(valid_conf(), true)) {
        CAPTURE(valid.test);
        REQUIRE_NOTHROW(parse(valid.test));
        //auto our_ast = parse(valid.test);
        // TODO: implement rendering
        // let's also check round-trip rendering
        //auto rendered = our_ast->render();
        //CAPTURE(rendered);
        //auto reparsed = parse(rendered);
        //REQUIRE(config_value_equal(our_ast, reparsed));
    }
}

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

TEST_CASE("implied comma handling") {
    auto valids = {
        R"(
// one line
{
  a : y, b : z, c : [ 1, 2, 3 ]
}
)",     R"(
// multiline but with all commas
{
  a : y,
  b : z,
  c : [
    1,
    2,
    3,
  ],
}
)",     R"(
// multiline with no commas
{
  a : y
  b : z
  c : [
    1
    2
    3
  ]
}
)"
    };

    auto drop_curlies = [](string const&s) {
        // drop the outside curly braces
        auto first = s.find('{');
        auto last = s.rfind('}');
        return s.substr(0, first) + s.substr(first+1, last-(first+1)) + s.substr(last+1);
    };

    auto changes = vector<function<string(string const&)>>{
        [](string const& s) { return s; },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, "\n", "\n\n"); },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, "\n", "\n\n\n"); },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, ",\n", "\n,\n"); },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, ",\n", "\n\n,\n\n"); },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, "\n", "\n "); },
        [](string const& s) { return boost::algorithm::replace_all_copy(s, ",\n", "  \n  \n  ,  \n  \n  "); },
        [&](string const& s) { return drop_curlies(s); }
    };

    auto tested = 0;
    for (auto v : valids) {
        for (auto change : changes) {
            ++tested;
            auto obj = parse_config(change(v));
            CAPTURE(v);
            REQUIRE(3u == get_size(obj->root()));
            REQUIRE("y" == obj->get_string("a"));
            REQUIRE("z" == obj->get_string("b"));
        }
    }

    REQUIRE((valids.size() * changes.size()) == tested);
}