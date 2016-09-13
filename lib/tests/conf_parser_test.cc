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
#include "test_utils.hpp"

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

static shared_value parse_without_resolving(string s) {
    auto options = config_parse_options()
        .set_origin_description(make_shared<string>("test conf string"))
        .set_syntax(config_syntax::CONF);
    return parseable::new_string(move(s), move(options))->parse_value();
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
        auto our_ast = parse(valid.test);
        // let's also check round-trip rendering
        auto rendered = our_ast->render();
        CAPTURE(rendered);
        auto reparsed = parse(rendered);
        REQUIRE(*our_ast == *reparsed);
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
)", R"(
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
)", R"(
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

    auto drop_curlies = [](string const &s) {
        // drop the outside curly braces
        auto first = s.find('{');
        auto last = s.rfind('}');
        return s.substr(0, first) + s.substr(first + 1, last - (first + 1)) + s.substr(last + 1);
    };

    auto changes = vector<function<string(string const &)>>{
        [](string const &s) { return s; },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, "\n", "\n\n"); },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, "\n", "\n\n\n"); },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, ",\n", "\n,\n"); },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, ",\n", "\n\n,\n\n"); },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, "\n", "\n "); },
        [](string const &s) { return boost::algorithm::replace_all_copy(s, ",\n", "  \n  \n  ,  \n  \n  "); },
        [&](string const &s) { return drop_curlies(s); }
    };

    auto tested = 0u;
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

    // TODO: move implied comma handling (pending) tests here once they pass

    auto no_newline_in_object = parse_config(" { a : b c } ");
    REQUIRE("b c" == no_newline_in_object->get_string("a"));

    auto no_newline_at_end = parse_config("a : b");
    REQUIRE("b" == no_newline_at_end->get_string("a"));

    REQUIRE_THROWS_AS(parse_config("{ a : y b : z }"), config_exception);

    REQUIRE_THROWS_AS(parse_config(R"({ "a" : "y" "b" : "z" })"), config_exception);

    // with no newline or comma, we do value concatenation
    auto no_newline_in_array = parse_config(" { c : [ 1 2 3 ] } ");
    REQUIRE(vector<string>{"1 2 3"} == no_newline_in_array->get_string_list("c"));

    auto no_newline_in_array_with_quoted = parse_config(R"( { c : [ "4" "5" "6" ] } )");
    REQUIRE(vector<string>{"4 5 6"} == no_newline_in_array_with_quoted->get_string_list("c"));
}

TEST_CASE("keys with slash") {
    auto obj = parse_config(R"(/a/b/c=42, x/y/z : 32)");
    REQUIRE(42 == obj->get_int("/a/b/c"));
    REQUIRE(32 == obj->get_int("x/y/z"));
}

static void line_number_test(int num, string text) {
    try {
        parse_config(text);
    } catch(config_exception &e) {
        cout << e.what() << endl;
        REQUIRE_STRING_CONTAINS(e.what(), to_string(num)+":");
    }
}

TEST_CASE("line numbers in errors (pending)", "[!shouldfail]") {
    // error is at the last char
    line_number_test(1, "}");
    line_number_test(2, "\n}");
    line_number_test(3, "\n\n}");

    // error is before a final newline
    line_number_test(1, "}\n");
    line_number_test(2, "\n}\n");
    line_number_test(3, "\n\n}\n");

    // with unquoted string
    line_number_test(1, "foo");
    line_number_test(2, "\nfoo");
    line_number_test(3, "\n\nfoo");

    // with quoted string
    line_number_test(1, "\"foo\"");
    line_number_test(2, "\n\"foo\"");
    line_number_test(3, "\n\n\"foo\"");

    // newlines in triple-quoted string should not hose up the numbering
    line_number_test(1, "a : \"\"\"foo\"\"\"}");
    line_number_test(2, "a : \"\"\"foo\n\"\"\"}");
    line_number_test(3, "a : \"\"\"foo\nbar\nbaz\"\"\"}");
    //   newlines after the triple quoted string
    line_number_test(5, "a : \"\"\"foo\nbar\nbaz\"\"\"\n\n}");
    //   triple quoted string ends in a newline
    line_number_test(6, "a : \"\"\"foo\nbar\nbaz\n\"\"\"\n\n}");
    //   end in the middle of triple-quoted string
    line_number_test(5, "a : \"\"\"foo\n\n\nbar\n");
}

TEST_CASE("to string for parseables") {
    // just be sure the to_string don't throw, to get test coverage
    auto options = config_parse_options();
    parseable::new_file("foo", options)->to_string();
    // TODO: are other APIs needed?
}

static void assert_comments(vector<string> comments, shared_config conf) {
    REQUIRE(comments == conf->root()->origin()->comments());
}

static void assert_comments(vector<string> comments, shared_config conf, string path) {
    REQUIRE(comments == conf->get_value(path)->origin()->comments());
}

static void assert_comments(vector<string> comments, shared_config conf, string path, int index) {
    // TODO:
    // auto v = conf->get_list(path)->get(index);
    // REQUIRE(comments == v->origin()->comments());
}

TEST_CASE("track comments for single field") {
    // no comments
    auto conf0 = parse_config(R"(
            {
            foo=10 }
            )");
    assert_comments({}, conf0, "foo");

    // comment in front of a field is used
    auto conf1 = parse_config(R"(
            { # Before
            foo=10 }
            )");
    assert_comments({" Before"}, conf1, "foo");

    // comment with a blank line after is dropped
    auto conf2 = parse_config(R"(
            { # BlankAfter

            foo=10 }
            )");
    assert_comments({}, conf2, "foo");

    // comment in front of a field is used with no root {}
    auto conf3 = parse_config(R"(
            # BeforeNoBraces
            foo=10
            )");
    assert_comments({" BeforeNoBraces"}, conf3, "foo");

    // comment with a blank line after is dropped with no root {}
    auto conf4 = parse_config(R"(
            # BlankAfterNoBraces

            foo=10
            )");
    assert_comments({}, conf4, "foo");

    // comment same line after field is used
    auto conf5 = parse_config(R"(
            {
            foo=10 # SameLine
            }
            )");
    assert_comments({" SameLine"}, conf5, "foo");

    // comment before field separator is used
    auto conf6 = parse_config(R"(
            {
            foo # BeforeSep
            =10
            }
            )");
    assert_comments({" BeforeSep"}, conf6, "foo");

    // comment after field separator is used
    auto conf7 = parse_config(R"(
            {
            foo= # AfterSep
            10
            }
            )");
    assert_comments({" AfterSep"}, conf7, "foo");

    // comment on next line is NOT used
    auto conf8 = parse_config(R"(
            {
            foo=10
            # NextLine
            }
            )");
    assert_comments({}, conf8, "foo");

    // comment before field separator on new line
    auto conf9 = parse_config(R"(
            {
            foo
            # BeforeSepOwnLine
            =10
            }
            )");
    assert_comments({" BeforeSepOwnLine"}, conf9, "foo");

    // comment after field separator on its own line
    auto conf10 = parse_config(R"(
            {
            foo=
            # AfterSepOwnLine
            10
            }
            )");
    assert_comments({" AfterSepOwnLine"}, conf10, "foo");

    // comments comments everywhere
    auto conf11 = parse_config(R"(
            {# Before
            foo
            # BeforeSep
            = # AfterSepSameLine
            # AfterSepNextLine
            10 # AfterValue
            # AfterValueNewLine (should NOT be used)
            }
            )");
    assert_comments({" Before", " BeforeSep", " AfterSepSameLine", " AfterSepNextLine", " AfterValue"}, conf11, "foo");

    // empty object
    auto conf12 = parse_config(R"(# BeforeEmpty
            {} #AfterEmpty
            # NewLine
            )");
    assert_comments({" BeforeEmpty", "AfterEmpty"}, conf12);

    // empty array
    auto conf13 = parse_config(R"(
            foo=
            # BeforeEmptyArray
            [] #AfterEmptyArray
            # NewLine
            )");
    assert_comments({" BeforeEmptyArray", "AfterEmptyArray"}, conf13, "foo");

    // array element
    auto conf14 = parse_config(R"(
            foo=[
            # BeforeElement
            10 # AfterElement
            ]
            )");
    assert_comments({" BeforeElement", " AfterElement"}, conf14, "foo", 0);

    // field with comma after it
    auto conf15 = parse_config(R"(
            foo=10, # AfterCommaField
            )");
    assert_comments({" AfterCommaField"}, conf15, "foo");

    // element with comma after it
    auto conf16 = parse_config(R"(
            foo=[10, # AfterCommaElement
            ]
            )");
    assert_comments({" AfterCommaElement"}, conf16, "foo", 0);

    // field with comma after it but comment isn't on the field's line, so not used
    auto conf17 = parse_config(R"(
            foo=10
            , # AfterCommaFieldNotUsed
            )");
    assert_comments({}, conf17, "foo");

    // element with comma after it but comment isn't on the field's line, so not used
    auto conf18 = parse_config(R"(
            foo=[10
            , # AfterCommaElementNotUsed
            ]
            )");
    assert_comments({}, conf18, "foo", 0);

    // comment on new line, before comma, should not be used
    auto conf19 = parse_config(R"(
            foo=10
            # BeforeCommaFieldNotUsed
            ,
            )");
    assert_comments({}, conf19, "foo");

    // comment on new line, before comma, should not be used
    auto conf20 = parse_config(R"(
            foo=[10
            # BeforeCommaElementNotUsed
            ,
            ]
            )");
    assert_comments({}, conf20, "foo", 0);

    // comment on same line before comma
    auto conf21 = parse_config(R"(
            foo=10 # BeforeCommaFieldSameLine
            ,
            )");
    assert_comments({" BeforeCommaFieldSameLine"}, conf21, "foo");

    // comment on same line before comma
    auto conf22 = parse_config(R"(
            foo=[10 # BeforeCommaElementSameLine
            ,
            ]
            )");
    assert_comments({" BeforeCommaElementSameLine"}, conf22, "foo", 0);

    // comment with a line containing only whitespace after is dropped
    auto conf23 = parse_config(R"(
            { # BlankAfter

            foo=10 }
            )");
    assert_comments({}, conf23, "foo");
}

TEST_CASE("track comments for multiple fields") {
    // nested objects
    auto conf5 = parse_config(R"(
            # Outside
            bar {
                # Ignore me

                # Middle
                # two lines
                baz {
                    # Inner
                    foo=10 # AfterInner
                    # This should be ignored
                } # AfterMiddle
                # ignored
            } # AfterOutside
            # ignored!
            )");
    assert_comments({" Inner", " AfterInner"}, conf5, "bar.baz.foo");
    assert_comments({" Middle", " two lines", " AfterMiddle"}, conf5, "bar.baz");
    assert_comments({" Outside", " AfterOutside"}, conf5, "bar");

    // multiple fields
    auto conf6 = parse_config(R"({
            # this is not with a field

            # this is field A
            a : 10,
            # this is field B
            b : 12 # goes with field B which has no comma
            # this is field C
            c : 14, # goes with field C after comma
            # not used
            # this is not used
            # nor is this
            # multi-line block

            # this is with field D
            # this is with field D also
            d : 16

            # this is after the fields
            })");
    assert_comments({" this is field A"}, conf6, "a");
    assert_comments({" this is field B", " goes with field B which has no comma"}, conf6, "b");
    assert_comments({" this is field C", " goes with field C after comma"}, conf6, "c");
    assert_comments({" this is with field D", " this is with field D also"}, conf6, "d");

    // array
    auto conf7 = parse_config(R"(
            # before entire array
            array = [
            # goes with 0
            0,
            # goes with 1
            1, # with 1 after comma
            # goes with 2
            2 # no comma after 2
            # not with anything
            ] # after entire array
            )");
    assert_comments({" goes with 0"}, conf7, "array", 0);
    assert_comments({" goes with 1", " with 1 after comma"}, conf7, "array", 1);
    assert_comments({" goes with 2", " no comma after 2"}, conf7, "array", 2);
    assert_comments({" before entire array", " after entire array"}, conf7, "array");
}

// TODO: this test used to fail due to an unimplemented method. Now it seems to reveal a bug in comment handling.
TEST_CASE("track comments for multiple fields (pending)", "[!shouldfail]") {
    // properties-like syntax
    auto conf8 = parse_config(R"(
            # ignored comment

            # x.y comment
            x.y = 10
            # x.z comment
            x.z = 11
            # x.a comment
            x.a = 12
            # a.b comment
            a.b = 14
            a.c = 15
            a.d = 16 # a.d comment
            # ignored comment
            )");

    assert_comments({" x.y comment"}, conf8, "x.y");
    assert_comments({" x.z comment"}, conf8, "x.z");
    assert_comments({" x.a comment"}, conf8, "x.a");
    assert_comments({" a.b comment"}, conf8, "a.b");
    assert_comments({}, conf8, "a.c");
    assert_comments({" a.d comment"}, conf8, "a.d");
    // here we're concerned that comments apply only to leaf
    // nodes, not to parent objects.

    // TODO: comments are currently being applied to the root as well. Why?
    assert_comments({}, conf8, "x");
    assert_comments({}, conf8, "a");
}

TEST_CASE("include file") {
    auto conf = config::parse_string("include file(\"" + fixture_path("test01") + "\")");

    // should have loaded conf, json
    REQUIRE(42u == conf->get_int("ints.fortyTwo"));
    REQUIRE(1u == conf->get_int("fromJson1"));
}

TEST_CASE("include file with extension") {
    auto conf = config::parse_string("include file(\"" + fixture_path("test01.conf") + "\")");

    REQUIRE(42u == conf->get_int("ints.fortyTwo"));
    REQUIRE_THROWS_AS(conf->get_int("fromJson1"), config_exception);
}

TEST_CASE("include file whitespace inside parens") {
    auto conf = config::parse_string("include file(  \n  \"" + fixture_path("test01") + "\"  \n  )");


    // should have loaded conf, json
    REQUIRE(42u == conf->get_int("ints.fortyTwo"));
    REQUIRE(1u == conf->get_int("fromJson1"));
}

TEST_CASE("include file no whitespace outside parens") {
    bool thrown = false;
    try {
        auto conf = config::parse_string("include file (\"" + fixture_path("test01") + "\")");
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "expecting include parameter");
    }
    REQUIRE(thrown);
}

TEST_CASE("include file not quoted") {
    bool thrown = false;
    try {
        auto conf = config::parse_string("include file(" + fixture_path("test01") + ")");
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "expecting include parameter");
    }
    REQUIRE(thrown);
}

TEST_CASE("include file not quoted and special char") {
    bool thrown = false;
    try {
        auto conf = config::parse_string("include file(:" + fixture_path("test01") + ")");
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "expecting a quoted string");
    }
    REQUIRE(thrown);
}

TEST_CASE("include file unclosed parens") {
    bool thrown = false;
    try {
        auto conf = config::parse_string("include file(" + fixture_path("test01"));
    } catch (const hocon::config_exception& e) {
        thrown = true;
        REQUIRE_STRING_CONTAINS(e.what(), "expecting include parameter");
    }
    REQUIRE(thrown);
}

// TODO: require URL support
// include url basename
// include url with extension
// include url invalid
// include resources?
// include url heuristically
// include url basename heuristically
// TODO: unicode support
// accept bom starting file
// accept bom start of string config
// accept bom in string value
// accept bom whitespace

TEST_CASE("accept multi period numeric path") {
    auto conf1 = config::parse_string("0.1.2.3=foobar1");
    REQUIRE("foobar1" == conf1->get_string("0.1.2.3"));
    auto conf2 = config::parse_string("0.1.2.3.ABC=foobar2");
    REQUIRE("foobar2" == conf2->get_string("0.1.2.3.ABC"));
    auto conf3 = config::parse_string("ABC.0.1.2.3=foobar3");
    REQUIRE("foobar3" == conf3->get_string("ABC.0.1.2.3"));
}
