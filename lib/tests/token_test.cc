#include <catch.hpp>

#include <internal/token.hpp>
#include <internal/tokens.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_boolean.hpp>
#include "test_utils.hpp"

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

TEST_CASE("token equality", "[tokens]") {

    SECTION("singleton token equality") {
        REQUIRE(*tokens::start_token() == *tokens::start_token());
        REQUIRE_FALSE(*tokens::start_token() == *tokens::end_token());
    }

    SECTION("value token equality") {
        value true_value(make_shared<config_boolean>(fake_origin("fake"), true));
        value other_true(make_shared<config_boolean>(fake_origin("other fake"), true));
        value false_value(make_shared<config_boolean>(fake_origin("fake"), false));

        REQUIRE(true_value == other_true);
        REQUIRE_FALSE(true_value == false_value);
    }

    SECTION("line token equality") {
        line line_value(fake_origin());
        line other_line(fake_origin("other fake"));
        ignored_whitespace not_a_line(fake_origin(), "   ");

        REQUIRE(line_value == other_line);
        REQUIRE_FALSE(line_value == not_a_line);
    }

    SECTION("unquoted text token equality") {
        unquoted_text text(fake_origin(), "no quotes");
        unquoted_text other_text(fake_origin("other fake"), "no quotes");
        unquoted_text different_text(fake_origin(), "still no quotes");
        ignored_whitespace not_unquoted_text(fake_origin(), "   ");

        REQUIRE(text == other_text);
        REQUIRE_FALSE(text == different_text);
        REQUIRE_FALSE(text == not_unquoted_text);
    }

    SECTION("ignored whitespace equality") {
        ignored_whitespace three_spaces(fake_origin(), "   ");
        ignored_whitespace three_more_spaces(fake_origin("other fake"), "   ");
        ignored_whitespace two_spaces(fake_origin(), "  ");
        unquoted_text not_whitespace(fake_origin(), "foo");

        REQUIRE(three_spaces == three_more_spaces);
        REQUIRE_FALSE(three_spaces == two_spaces);
        REQUIRE_FALSE(three_spaces == not_whitespace);
    }

    SECTION("problem token equality") {
        problem problem_token(fake_origin(), "test exception", "it broke", false);
        problem other_problem(fake_origin("other fake"), "test exception", "it broke", false);
        ignored_whitespace not_a_problem(fake_origin(), "   ");

        REQUIRE(problem_token == other_problem);
        REQUIRE_FALSE(problem_token == not_a_problem);
    }

    SECTION("comment equality") {
        comment comment_token(fake_origin(), "my comment");
        comment other_comment(fake_origin("other fake"), "my comment");
        comment different_comment(fake_origin(), "a different comment");
        ignored_whitespace not_a_comment(fake_origin(), "   ");

        REQUIRE(comment_token == other_comment);
        REQUIRE_FALSE(comment_token == different_comment);
        REQUIRE_FALSE(comment_token == not_a_comment);

        // Comment comparison only deals with the actual text, not the marker
        hash_comment hash(fake_origin(), "my comment");
        double_slash_comment slashes(fake_origin("other origin"), "my comment");

        REQUIRE(hash == slashes);
    }

    SECTION("substitution equality") {
        token_list expression1;
        expression1.push_back(make_shared<line>(fake_origin()));
        expression1.push_back(make_shared<line>(fake_origin("other")));

        token_list expression1_dup;
        expression1_dup.push_back(make_shared<line>(fake_origin()));
        expression1_dup.push_back(make_shared<line>(fake_origin("other")));

        token_list expression2;
        expression2.push_back(make_shared<line>(fake_origin()));

        substitution sub(fake_origin(), false, move(expression1));
        substitution other_sub(fake_origin("other"), false, move(expression1_dup));
        substitution different_sub(fake_origin(), false, move(expression2));
        line not_a_sub(fake_origin("line"));

        REQUIRE(sub == other_sub);
        REQUIRE_FALSE(sub == different_sub);
        REQUIRE_FALSE(sub == not_a_sub);
    }
}
