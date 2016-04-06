#include <catch.hpp>

#include <hocon/config.hpp>
#include <hocon/config_exception.hpp>
#include "test_utils.hpp"

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

    // TODO: noSubstitutionsListConcat
    // TODO: listConcatWithSubstatutions
    // TODO: listConcatSelfReferential
    // TODO: listConcatCanSpanLinesInsideBrackets
    // We really need to flesh out our data access story before it's
    // worth writing these, as the API could change dramatically

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

    // TODO: noSubstitutionsObjectConcat
    // TODO: objectConcatMergeOrder
    // TODO: objectConcatWithSubstitutions
    // TODO: objectConcatSelfReferential
    // TODO: objectConcatSelfReferentialOverride
    // TODO: objectConcatCanSpanLinesInsideBraces
    // As above, I don't want to write these until we know how we're
    // getting structured data out of HOCON

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
    // TOOD: objectConcatInsideArrayValue
    // TODO: objectNonConcatInsideArrayValue
    // TOOD: listConcatInsideArrayValue
    // TOOD: listNonConcatInsideArrayValue
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

}

TEST_CASE("Concatenation pending system environment lookup implementation", "[!shouldfail]") {
    // We expect a lookup to fail and fall through in these tests.
    // Unfortunately, because system environment lookup is not yet
    // implemented, we fall through to that code path and then explode
    // with an exception.
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
            printf(e.what());
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
