#include <catch.hpp>

#include <hocon/config.hpp>
#include "test_utils.hpp"
#include <hocon/config_list.hpp>

using namespace hocon;
using namespace hocon::test_utils;

TEST_CASE("concatenation") {

    SECTION("no substitutions string concatenation") {
        auto conf = parse_config(R"(a :  true "xyz" 123 foo)")->resolve();
        REQUIRE(conf->get_string("a") == "true xyz 123 foo");
    }

    SECTION("trivial string concatenation") {
        auto conf = parse_config(R"(a : ${x}foo, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1foo");
    }

    SECTION("two substitution string concatenation") {
        auto conf = parse_config(R"(a : ${x}foo${x}, x = 1)")->resolve();
        REQUIRE(conf->get_string("a") == "1foo1");
    }

    SECTION("string concatenation cannot span lines") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a : ${x}
                                        foo, x = 1)")->resolve();
        } catch (const hocon::parse_exception& e) {
            thrown = true;
            REQUIRE(std::string(e.what()).find("not be followed") != std::string::npos);
        }
        REQUIRE(thrown);
    }

    SECTION("string concatenation cannot contain objects") {
        bool thrown = false;
        try {
            auto conf = parse_config("R(a : abc { x : y })")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "abc");
            // TODO: Add this when we can properly print subexpressions from exceptions
            // REQUIRE_STRING_CONTAINS(e.what(), R"({"X":"Y"})");
        }
        REQUIRE(thrown);
    }

    SECTION("object concatenation cannot contain null") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a : null { x : y })")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "null");
            // TODO: Add this when we can properly print subexpressions from exceptions
            // REQUIRE_STRING_CONTAINS(e.what(), R"({"X":"Y"})");
        }
        REQUIRE(thrown);
    }

    SECTION("string concatenation cannot contain arrays") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a : abc [1, 2])")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "abc");
            // TODO: Add this when we can properly print subexpressions from exceptions
            // REQUIRE_STRING_CONTAINS(e.what(), "[1,2]");
        }
        REQUIRE(thrown);
    }

    SECTION("string concatenation cannot contain objects via substitution") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a : abc ${x}, x: { y : z })")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "abc");
            // TODO: Add this when we can properly print subexpressions in exceptions
            // REQUIRE_STRING_CONTAINS(e.what(), R"({"X":"Y"})");
        }
        REQUIRE(thrown);

    }

    SECTION("string concatenation cannot contain arrays via substitution") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a : abc ${x}, x: [1, 2])")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "abc");
            // TODO: Add this when we can properly print subexpressions in exceptions
            // REQUIRE_STRING_CONTAINS(e.what(), "[1,2]");
        }
        REQUIRE(thrown);
    }

    SECTION("list concatenation with no substitutions") {
        auto conf = parse_config(" a :  [1,2] [3,4]  ");
        std::vector<unwrapped_value> expected { 1,2,3,4 };
        unwrapped_value v(expected);
        bool test = v == conf->get_list("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("list concatenation with substitutions") {
        auto conf = parse_config(" a :  ${x} [3,4] ${y}, x : [1,2], y : [5,6]  ")->resolve();
        std::vector<unwrapped_value> expected {1,2,3,4,5,6};
        unwrapped_value v(expected);
        bool test = v == conf->get_list("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("list concatenation with self-references") {
        auto conf = parse_config(" a : [1, 2], a : ${a} [3,4], a : ${a} [5,6]  ")->resolve();
        std::vector<unwrapped_value> expected {1,2,3,4,5,6};
        unwrapped_value v(expected);
        bool test = v == conf->get_list("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("list concat can span lines inside brackets") {
        auto conf = parse_config(" a :  [1,2\n] [3,4]  ");
        std::vector<unwrapped_value> v {1,2,3,4};
        unwrapped_value expected(v);
        bool test = expected == conf->get_list("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("list concatenation cannot span lines") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a: [1,2]
                                        [3,4])")->resolve();
        } catch (const hocon::parse_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "expecting");
            REQUIRE_STRING_CONTAINS(e.what(), "'['");
        }
        REQUIRE(thrown);
    }

    SECTION("object concatenation without substitutions") {
        auto conf = parse_config(" a : { b : c } { x : y }  ");
        std::unordered_map<std::string, unwrapped_value> m {{"b", std::string("c")}, {"x", std::string("y")}};
        unwrapped_value expected(m);
        bool test = expected == conf->get_object("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("object concatenation should preserve merge order") {
        auto conf = parse_config(" a : { b : 1 } { b : 2 } { b : 3 } { b : 4 } ");
        REQUIRE(4 == conf->get_int("a.b"));
    }

    SECTION("object concatenation with substitutions") {
        auto conf = parse_config(" a : ${x} { b : 1 } ${y}, x : { a : 0 }, y : { c : 2 } ")->resolve();
        std::unordered_map<std::string, unwrapped_value> m {{"a", 0}, {"b", 1}, {"c", 2}};
        unwrapped_value expected(m);
        bool test = expected == conf->get_object("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("object concatenation with self-references") {
        auto conf = parse_config(" a : { a : 0 }, a : ${a} { b : 1 }, a : ${a} { c : 2 } ")->resolve();
        std::unordered_map<std::string, unwrapped_value> m {{"a", 0}, {"b", 1}, {"c", 2}};
        unwrapped_value expected(m);
        bool test = expected == conf->get_object("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("object concatenation self-reference override") {
        auto conf = parse_config(" a : { b : 3 }, a : { b : 2 } ${a} ")->resolve();
        std::unordered_map<std::string, unwrapped_value> m {{"b", 3}};
        unwrapped_value expected(m);
        bool test = expected == conf->get_object("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("object concatenation can span lines inside braces") {
        auto conf = parse_config(" a :  { b : c\n } { x : y }  ");
        std::unordered_map<std::string, unwrapped_value> m {{"b", std::string("c")}, {"x", std::string("y")}};
        unwrapped_value expected(m);
        bool test = expected == conf->get_object("a")->unwrapped();
        REQUIRE(test);
    }

    SECTION("object concatenation cannot span lines") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(A : { b : c}
                                        {x : y })")->resolve();
        } catch (const hocon::parse_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "expecting");
            REQUIRE_STRING_CONTAINS(e.what(), "'{'");
        }
        REQUIRE(thrown);
    }

    // TODO: stringConcatInsideArrayValue
    // TODO: stringNonConcatInsideArrayValue
    // TODO: objectConcatInsideArrayValue
    // TODO: objectNonConcatInsideArrayValue

    SECTION("list concatenation inside an array") {
        auto conf = parse_config(" a : [ [1, 2] [3, 4] ] ");
        std::vector<unwrapped_value> inner {1,2,3,4};
        std::vector<unwrapped_value> outer;
        outer.emplace_back(inner);
        unwrapped_value expected(outer);
        bool test = expected == conf->get_list("a")->unwrapped();
        REQUIRE(test);
    };
    // TODO: listNonConcatInsideArrayValue
    // These all require we have the data access methods fleshed out

    SECTION("string concatenations can be keys") {
        auto conf = parse_config(R"(123 foo : "value" )")->resolve();
        REQUIRE(conf->get_string("123 foo") == "value");
    }

    SECTION("objects are not keys") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"({ { a : 1 } : "value")")->resolve();
        } catch (const hocon::parse_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "expecting a close");
            REQUIRE_STRING_CONTAINS(e.what(), "'{'");
        }
        REQUIRE(thrown);
    }

    SECTION("arrays are not keys") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"({ [ 1, 2 ] : "value")")->resolve();
        } catch (const hocon::parse_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "expecting a close");
            REQUIRE_STRING_CONTAINS(e.what(), "'['");
        }
        REQUIRE(thrown);
    }

    // TODO: '+=' pending tests go here

    // TODO: arrayConcatenationInDoubleNestedDelayedMerge
    // TODO: arrayConcatenationAsPartOfDelayedMerge
    // TODO: arrayConcatenationInDoubleNestedDelayedMerge2
    // TODO: arrayConcatenationInTripleNestedDelayedMerge
    // Again, waiting on good accessors for lists

    SECTION("string concatenation with defined optional substitution") {
        auto conf = parse_config("bar=bar, a = foo${?bar}")->resolve();
        REQUIRE(conf->get_string("a") == "foobar");
    }

    // TODO: concatUndefinedSubstitutionWithArray
    // TODO: concatDefinedOptionalSubstitutionWithArray
    // Guess why?

    SECTION("object concatenation with defined optional substitution") {
        auto conf = parse_config(R"(bar={ y : 42 }, a = { x : "foo" } ${?bar})")->resolve();
        REQUIRE(conf->get_string("a.x") == "foo");
        REQUIRE(conf->get_int("a.y") == 42);
    }

    SECTION("concatenate two undefined substitutions with a space") {
        auto conf = parse_config(R"(foo=abc, bar=def, a = ${foo} ${bar})")->resolve();
        REQUIRE(conf->get_string("a") == "abc def");
    }

    SECTION("concatenate object substitutions with no space") {
        auto conf = parse_config(R"(foo = { a : 1}, bar = { b : 2 }, x = ${foo}${bar})")->resolve();
        REQUIRE(conf->get_int("x.a") == 1);
        REQUIRE(conf->get_int("x.b") == 2);
    }

    SECTION("concatenate object substitutions with a space") {
        auto conf = parse_config(R"(foo = { a : 1}, bar = { b : 2 }, x = ${foo} ${bar})")->resolve();
        REQUIRE(conf->get_int("x.a") == 1);
        REQUIRE(conf->get_int("x.b") == 2);
    }

    SECTION("concatenate object substitutions with quoted space") {
        REQUIRE_THROWS_AS(
            parse_config(R"(foo = { a : 1}, bar = { b : 2 }, x = ${foo}"  "${bar})")->resolve(),
            config_exception
        );
    }

    // TODO: concatSubstitutionsThatAreListsWithSpace

    SECTION("concatenate list substitutions with quoted space") {
        REQUIRE_THROWS_AS(
            parse_config(R"(foo = [1], bar = [2], x = ${foo}"  "${bar})")->resolve(),
            config_exception
        );
    }

    SECTION("string concatenation with undefined substitution") {
        auto conf = parse_config("a = foo${?bar}")->resolve();
        REQUIRE(conf->get_string("a") == "foo");
    }

    SECTION("object concatenation with undefined substitution") {
        auto conf = parse_config(R"(a = { x : "foo" } ${?bar})")->resolve();
        REQUIRE(conf->get_string("a.x") == "foo");
    }

    SECTION("concatenate two undefined substitutions") {
        auto conf = parse_config(R"(a = ${?foo}${?bar})")->resolve();
        REQUIRE_FALSE(conf->has_path("a"));
    }

    SECTION("concatenate several undefined substitusion") {
        auto conf = parse_config(R"(a = ${?foo}${?bar}${?baz}${?woooo})")->resolve();
        REQUIRE_FALSE(conf->has_path("a"));
    }

    SECTION("concatenate two undefined substitutions with a space") {
        auto conf = parse_config(R"(a = ${?foo} ${?bar})")->resolve();
        REQUIRE(conf->get_string("a") == " ");
    }

    SECTION("concatenate two undefined substitutions with an empty string") {
        auto conf = parse_config(R"(a = ""${?foo}${?bar})")->resolve();
        REQUIRE(conf->get_string("a") == "");
    }
}

TEST_CASE("Concatenation pending '+=' implementation", "[!shouldfail]") {
    // TODO: emptyArrayPlusEquals
    // TODO: missingArrayPlusEquals
    // TODO: shortArrayPlusEquals
    // You guessed it, these need a good story around pulling arrays out

    SECTION("can't concatenate numbers as an array using +=") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a = 10, a += 2)")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "10");
            REQUIRE_STRING_CONTAINS(e.what(), "[2]");
        }
        REQUIRE(thrown);
    }

    SECTION("can't concatenate strings with an array using +=") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a = "abc", a += 2)")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), "abc");
            REQUIRE_STRING_CONTAINS(e.what(), "[2]");
        }
        REQUIRE(thrown);
    }

    SECTION("can't concatenate objectss with an array using +=") {
        bool thrown = false;
        try {
            auto conf = parse_config(R"(a = { x : y }, a += 2)")->resolve();
        } catch (const hocon::config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Cannot concatenate");
            REQUIRE_STRING_CONTAINS(e.what(), R"("x":"y")");
            REQUIRE_STRING_CONTAINS(e.what(), "[2]");
        }
        REQUIRE(thrown);
    }

    // TODO: plusEqualsNestedPath
    // TODO: plusEqualsNestedObjects
    // TODO: plusEqualsSingleNestedObject
    // TOOD: substitutionPlusEqualsSubstitution
    // TODO: plusEqualsMultipleTimes
    // TOOD: plusEqualsMultipleTimesNested
    // TODO: plusEqualsAnObjectMultipleTimes
    // TODO: plusEqualsAnObjectMultipleTimesNested
    // TOOD: plusEqualsMultipleTimesNestedInArray
    // TODO: plusEqualsMultipleTimesNestedInPlusEquals
}
