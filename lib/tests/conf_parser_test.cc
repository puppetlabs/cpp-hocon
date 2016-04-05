#include <catch.hpp>
#include "test_utils.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <hocon/config.hpp>
#include <hocon/config_list.hpp>
#include <hocon/config_exception.hpp>
#include <hocon/config_parse_options.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/values/config_reference.hpp>
#include <internal/substitution_expression.hpp>
#include <internal/parseable.hpp>
#include <internal/resolve_context.hpp>
#include <internal/path_parser.hpp>

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
        //REQUIRE(*our_ast == *reparsed);
    }
}

static path parse_path(string s) {
    // parse first by wrapping into a whole document and using the regular parser.
    shared_value tree;
    CAPTURE(s);
    REQUIRE_NOTHROW(tree = parse_without_resolving("[${"+s+"}]"));
    path result = [&]() {
        if (auto list = dynamic_pointer_cast<const config_list>(tree)) {
            if (auto ref = dynamic_pointer_cast<const config_reference>(list->get(0))) {
                return ref->expression()->get_path();
            }
        }
        return path();
    }();

    // also parse with the standalone path parser and be sure the
    // outcome is the same
    path should_be_same;
    REQUIRE_NOTHROW(should_be_same = path_parser::parse_path(s));
    REQUIRE(result == should_be_same);

    return result;
}

TEST_CASE("path parsing") {
    REQUIRE(path({"a"}) == parse_path("a"));
    REQUIRE(path(vector<string>{"a", "b"}) == parse_path("a.b"));
    REQUIRE(path({"a.b"}) == parse_path("\"a.b\""));
    REQUIRE(path({"a."}) == parse_path("\"a.\""));
    REQUIRE(path({".b"}) == parse_path("\".b\""));
    REQUIRE(path({"true"}) == parse_path("true"));
    REQUIRE(path({"a"}) == parse_path(" a "));
    REQUIRE(path(vector<string>{"a ", "b"}) == parse_path(" a .b"));
    REQUIRE(path(vector<string>{"a ", " b"}) == parse_path(" a . b"));
    REQUIRE(path({"a  b"}) == parse_path(" a  b"));
    REQUIRE(path(vector<string>{"a", "b.c", "d"}) == parse_path("a.\"b.c\".d"));
    REQUIRE(path(vector<string>{"3", "14"}) == parse_path("3.14"));
    REQUIRE(path(vector<string>{"3", "14", "159"}) == parse_path("3.14.159"));
    REQUIRE(path(vector<string>{"a3", "14"}) == parse_path("a3.14"));
    REQUIRE(path({""}) == parse_path("\"\""));
    REQUIRE(path(vector<string>{"a", "", "b"}) == parse_path("a.\"\".b"));
    REQUIRE(path(vector<string>{"a", ""}) == parse_path("a.\"\""));
    REQUIRE(path(vector<string>{"", "b"}) == parse_path("\"\".b"));
    REQUIRE(path(vector<string>{"", "", ""}) == parse_path(R"( "".""."" )"));
    REQUIRE(path({"a-c"}) == parse_path("a-c"));
    REQUIRE(path({"a_c"}) == parse_path("a_c"));
    REQUIRE(path({"-"}) == parse_path("\"-\""));
    REQUIRE(path({"-"}) == parse_path("-"));
    REQUIRE(path({"-foo"}) == parse_path("-foo"));
    REQUIRE(path({"-10"}) == parse_path("-10"));

    // here 10.0 is part of an unquoted string;
    REQUIRE(path(vector<string>{"foo10", "0"}) == parse_path("foo10.0"));
    // here 10.0 is a number that gets value-concatenated;
    REQUIRE(path(vector<string>{"10", "0foo"}) == parse_path("10.0foo"));
    // just a number;
    REQUIRE(path(vector<string>{"10", "0"}) == parse_path("10.0"));
    // multiple-decimal number;
    REQUIRE(path(vector<string>{"1", "2", "3", "4"}) == parse_path("1.2.3.4"));

    for (string invalid : {"", " ", "  \n   \n  ", "a.", ".b", "a..b", "a${b}c", "\"\".", ".\"\""}) {
        REQUIRE_THROWS_AS(parse_without_resolving("[${"+invalid+"}]"), bad_path_exception);
        REQUIRE_THROWS_AS(path_parser::parse_path(invalid), bad_path_exception);
    }
}

TEST_CASE("duplicate key last wins") {
    auto obj = parse_config(R"({ "a" : 10, "a" : 11 } )");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(11 == obj->get_int("a"));
}

TEST_CASE("duplicate key objects merged") {
    auto obj = parse_config(R"({ "a" : { "x" : 1, "y" : 2 }, "a" : { "x" : 42, "z" : 100 } })");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(3u == obj->get_object("a")->size());
    REQUIRE(42 == obj->get_int("a.x"));
    REQUIRE(2 == obj->get_int("a.y"));
    REQUIRE(100 == obj->get_int("a.z"));
}

TEST_CASE("duplicate key objects merged recursively") {
    auto obj = parse_config(R"({ "a" : { "b" : { "x" : 1, "y" : 2 } }, "a" : { "b" : { "x" : 42, "z" : 100 } } })");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(1u == obj->get_object("a")->size());
    REQUIRE(3u == obj->get_object("a.b")->size());
    REQUIRE(42 == obj->get_int("a.b.x"));
    REQUIRE(2 == obj->get_int("a.b.y"));
    REQUIRE(100 == obj->get_int("a.b.z"));
}

TEST_CASE("duplicate key objects merged recursively deeper") {
    auto obj = parse_config(R"({ "a" : { "b" : { "c" : { "x" : 1, "y" : 2 } } }, "a" : { "b" : { "c" : { "x" : 42, "z" : 100 } } } })");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(1u == obj->get_object("a")->size());
    REQUIRE(1u == obj->get_object("a.b")->size());
    REQUIRE(3u == obj->get_object("a.b.c")->size());
    REQUIRE(42 == obj->get_int("a.b.c.x"));
    REQUIRE(2 == obj->get_int("a.b.c.y"));
    REQUIRE(100 == obj->get_int("a.b.c.z"));
}

TEST_CASE("duplicate key object null object") {
    auto obj = parse_config(R"({ a : { b : 1 }, a : null, a : { c : 2 } })");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(1u == obj->get_object("a")->size());
    REQUIRE(2 == obj->get_int("a.c"));
}

TEST_CASE("duplicate key object number object") {
    auto obj = parse_config(R"({ a : { b : 1 }, a : 42, a : { c : 2 } })");
    REQUIRE(1u == obj->root()->size());
    REQUIRE(1u == obj->get_object("a")->size());
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
            REQUIRE(3u == obj->root()->size());
            REQUIRE("y" == obj->get_string("a"));
            REQUIRE("z" == obj->get_string("b"));
        }
    }

    REQUIRE((valids.size() * changes.size()) == tested);

    // with no newline or comma, we do value concatenation
    auto no_newline_in_array = parse_config(" { c : [ 1 2 3 ] } ");
}