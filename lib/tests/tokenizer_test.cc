#include <catch.hpp>
#include <internal/tokenizer.hpp>
#include "test_utils.hpp"
#include <iostream>

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

token_list tokenize_as_list(string const& source) {
    token_iterator iter(fake_origin(), unique_ptr<istringstream>(new istringstream(source)), true);
    // get all the tokens from the string and put them in a vector
    token_list tokens;
    while (iter.has_next()) {
        tokens.push_back(iter.next());
    }
    return tokens;
}

void tokenizer_test(string source, token_list expected) {
    token_list result = tokenize_as_list(source);
    for (size_t i = 0; i < expected.size(); i++) {
        if (!(*expected[i] == *result[i])) {
            // Debugging info
            cerr << result[i]->to_string() << " but expected " << expected[i]->to_string() << " -- ";
            cerr << "i = " + std::to_string(i);
        }
        REQUIRE(*expected[i] == *result[i]);
    }
}

TEST_CASE("tokenize basic strings", "[tokenizer]") {
    SECTION("tokenize empty string") {
        string source = "";
        token_list expected {
                tokens::start_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("tokenize newlines") {
        string source = "\n\n";
        token_list expected {
                tokens::start_token(),
                make_shared<line>(fake_origin()->with_line_number(1)),
                make_shared<line>(fake_origin()->with_line_number(2)),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("tokenize all types", "[tokenizer]") {
    SECTION("tokenize all types with no spaces") {
        string source = ",:=}{][+=\"foo\"\"\"\"bar\"\"\"true3.14false42null${a.b}${?x.y}${\"c.d\"}\n";
        token_list expected {
                tokens::start_token(),
                tokens::comma_token(),
                tokens::colon_token(),
                tokens::equals_token(),
                tokens::close_curly_token(),
                tokens::open_curly_token(),
                tokens::close_square_token(),
                tokens::open_square_token(),
                tokens::plus_equals_token(),
                string_token("foo"),
                string_token("bar"),
                bool_token(true),
                double_token(3.14, "3.14"),
                bool_token(false),
                int_token(42, "42"),
                null_token(),
                substitution_token(unquoted_text_token("a.b"), false),
                substitution_token(unquoted_text_token("x.y"), true),
                substitution_token(string_token("\"c.d\""), false),
                line_token(1),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("tokenize all types with spaces") {
        string source = " , : = } { ] [ += \"foo\" \"\"\"bar\"\"\" 42 true 3.14 false null ${a.b} ${?x.y} ${\"c.d\"} \n ";
        token_list expected {
                tokens::start_token(),
                whitespace_token(" "),
                tokens::comma_token(),
                whitespace_token(" "),
                tokens::colon_token(),
                whitespace_token(" "),
                tokens::equals_token(),
                whitespace_token(" "),
                tokens::close_curly_token(),
                whitespace_token(" "),
                tokens::open_curly_token(),
                whitespace_token(" "),
                tokens::close_square_token(),
                whitespace_token(" "),
                tokens::open_square_token(),
                whitespace_token(" "),
                tokens::plus_equals_token(),
                whitespace_token(" "),
                string_token("foo"),
                unquoted_text_token(" "),
                string_token("bar"),
                unquoted_text_token(" "),
                int_token(42, "42"),
                unquoted_text_token(" "),
                bool_token(true),
                unquoted_text_token(" "),
                double_token(3.14, "3.14"),
                unquoted_text_token(" "),
                bool_token(false),
                unquoted_text_token(" "),
                null_token(),
                unquoted_text_token(" "),
                substitution_token(unquoted_text_token("a.b"), false),
                unquoted_text_token(" "),
                substitution_token(unquoted_text_token("x.y"), true),
                unquoted_text_token(" "),
                substitution_token(string_token("\"c.d\""), false),
                whitespace_token(" "),
                line_token(1),
                whitespace_token(" "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("tokenize all types with multiple spaces") {
        string source = "   ,   :   =   }   {   ]   [   +=   \"foo\"   \"\"\"bar\"\"\"   42   true   3.14   false   null   ${a.b}   ${?x.y}   ${\"c.d\"}  \n   ";
        token_list expected {
                tokens::start_token(),
                whitespace_token("   "),
                tokens::comma_token(),
                whitespace_token("   "),
                tokens::colon_token(),
                whitespace_token("   "),
                tokens::equals_token(),
                whitespace_token("   "),
                tokens::close_curly_token(),
                whitespace_token("   "),
                tokens::open_curly_token(),
                whitespace_token("   "),
                tokens::close_square_token(),
                whitespace_token("   "),
                tokens::open_square_token(),
                whitespace_token("   "),
                tokens::plus_equals_token(),
                whitespace_token("   "),
                string_token("foo"),
                unquoted_text_token("   "),
                string_token("bar"),
                unquoted_text_token("   "),
                int_token(42, "42"),
                unquoted_text_token("   "),
                bool_token(true),
                unquoted_text_token("   "),
                double_token(3.14, "3.14"),
                unquoted_text_token("   "),
                bool_token(false),
                unquoted_text_token("   "),
                null_token(),
                unquoted_text_token("   "),
                substitution_token(unquoted_text_token("a.b"), false),
                unquoted_text_token("   "),
                substitution_token(unquoted_text_token("x.y"), true),
                unquoted_text_token("   "),
                substitution_token(string_token("\"c.d\""), false),
                whitespace_token("  "),
                line_token(1),
                whitespace_token("   "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("unquoted text and booleans", "[tokenizer]") {
    SECTION("true and unquoted text") {
        string source = "truefoo";
        token_list expected {
                tokens::start_token(),
                bool_token(true),
                unquoted_text_token("foo"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("false and unquoted text") {
        string source = "falsefoo";
        token_list expected {
                tokens::start_token(),
                bool_token(false),
                unquoted_text_token("foo"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("null and unquoted text") {
        string source = "nullfoo";
        token_list expected {
                tokens::start_token(),
                null_token(),
                unquoted_text_token("foo"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("unquoted text containing true") {
        string source = "footrue";
        token_list expected {
                tokens::start_token(),
                unquoted_text_token("footrue"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("unquoted text containing space true") {
        string source = "foo true";
        token_list expected {
                tokens::start_token(),
                unquoted_text_token("foo"),
                unquoted_text_token(" "),
                bool_token(true),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("true and space and unquoted text") {
        string source = "true foo";
        token_list expected {
                tokens::start_token(),
                bool_token(true),
                unquoted_text_token(" "),
                unquoted_text_token("foo"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("unquoted strings with special cases", "[tokenizer]") {
    SECTION("unquoted text containing slash") {
        string source = "a/b/c";
        token_list expected {
                tokens::start_token(),
                unquoted_text_token("a/b/c"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);

        source = "/";
        expected = {
                tokens::start_token(),
                unquoted_text_token("/"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);

        source = "/ /";
        expected = {
                tokens::start_token(),
                unquoted_text_token("/"),
                unquoted_text_token(" "),
                unquoted_text_token("/"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("unquoted text discards external spaces") {
        string source = "   foo   \n";
        token_list expected {
                tokens::start_token(),
                whitespace_token("   "),
                unquoted_text_token("foo"),
                whitespace_token("   "),
                line_token(1),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("unquoted text keeps internal spaces") {
        string source = "    foo  bar baz   \n";
        token_list expected {
                tokens::start_token(),
                whitespace_token("    "),
                unquoted_text_token("foo"),
                unquoted_text_token("  "),
                unquoted_text_token("bar"),
                unquoted_text_token(" "),
                unquoted_text_token("baz"),
                whitespace_token("   "),
                line_token(1),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("mix quoted and unquoted") {
        string source = "   foo\"bar\"baz   \n";
        token_list expected {
                tokens::start_token(),
                whitespace_token("   "),
                unquoted_text_token("foo"),
                string_token("bar"),
                unquoted_text_token("baz"),
                whitespace_token("   "),
                line_token(1),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("escape sequence", "[tokenizer]") {
    SECTION("unicode infinity symbol") {
        string source = "\"\\u221E\"";
        token_list expected {
                tokens::start_token(),
                string_token(u8"\u221E"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("null byte") {
        string source = " \"\\u0000\" ";
        token_list expected {
                tokens::start_token(),
                whitespace_token(" "),
                string_token(""),
                whitespace_token(" "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("various escape codes") {
        string source = " \"\\\"\\\\/\\b\\f\\n\\r\\t\" ";
        token_list expected {
                tokens::start_token(),
                whitespace_token(" "),
                string_token("\"\\/\b\f\n\r\t"),
                whitespace_token(" "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("unicode F") {
        string source = " \"\\u0046\" ";
        token_list expected {
                tokens::start_token(),
                whitespace_token(" "),
                string_token("F"),
                whitespace_token(" "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("two unicode F's") {
        string source = " \"\\u0046\\u0046\" ";
        token_list expected {
                tokens::start_token(),
                whitespace_token(" "),
                string_token("FF"),
                whitespace_token(" "),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("triple quoted strings") {
    SECTION("trivial triple quoted string") {
        string source = "\"\"\"bar\"\"\"";
        token_list expected {
                tokens::start_token(),
                string_token("bar"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("trailing quotes in triple quoted string") {
        string source = "\"\"\"\"\"\"\"\"";
        token_list expected {
                tokens::start_token(),
                string_token("\"\""),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("no esacpe in triple quoted strings") {
        string source = "\"\"\"\\n\"\"\"";
        token_list expected {
                tokens::start_token(),
                string_token("\\n"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("new line in triple quoted string") {
        string source = "\"\"\"foo\nbar\"\"\"";
        token_list expected {
                tokens::start_token(),
                string_token("foo\nbar"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("comments", "[tokenizer]") {
    SECTION("double slash comment") {
        string source = "//";
        token_list expected {
                tokens::start_token(),
                double_slash_comment_token(""),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash comment") {
        string source = "#";
        token_list expected {
                tokens::start_token(),
                hash_comment_token(""),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("two slashes in quoted string is string") {
        string source = "\"//bar\"";
        token_list expected {
                tokens::start_token(),
                string_token("//bar"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash in quoted string is string") {
        string source = "\"#bar\"";
        token_list expected {
                tokens::start_token(),
                string_token("#bar"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("slash comment after unquoted text") {
        string source = "bar//comment";
        token_list expected {
                tokens::start_token(),
                unquoted_text_token("bar"),
                double_slash_comment_token("comment"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash comment after unquoted text") {
        string source = "bar#comment";
        token_list expected {
                tokens::start_token(),
                unquoted_text_token("bar"),
                hash_comment_token("comment"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("slash comment after int") {
        string source = "10//comment";
        token_list expected {
                tokens::start_token(),
                int_token(10, "10"),
                double_slash_comment_token("comment"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash comment after int") {
        string source = "10#comment";
        token_list expected {
                tokens::start_token(),
                int_token(10, "10"),
                hash_comment_token("comment"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("slash comment with newline") {
        string source = "10//comment\n12";
        token_list expected {
                tokens::start_token(),
                int_token(10, "10"),
                double_slash_comment_token("comment"),
                line_token(1),
                int_token(12, "12"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash comment with newline") {
        string source = "10#comment\n12";
        token_list expected {
                tokens::start_token(),
                int_token(10, "10"),
                hash_comment_token("comment"),
                line_token(1),
                int_token(12, "12"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("slash comment on multiples lines with whitespace") {
        string source = "   //comment\r\n   //comment2   \n//comment3   \n\n//comment4";
        token_list expected {
                tokens::start_token(),
                whitespace_token("   "),
                double_slash_comment_token("comment\r"),
                line_token(1),
                whitespace_token("   "),
                double_slash_comment_token("comment2   "),
                line_token(2),
                double_slash_comment_token("comment3   "),
                line_token(3),
                line_token(4),
                double_slash_comment_token("comment4"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("hash comment on multiples lines with whitespace") {
        string source = "   #comment\r\n   #comment2   \n#comment3   \n\n#comment4";
        token_list expected {
                tokens::start_token(),
                whitespace_token("   "),
                hash_comment_token("comment\r"),
                line_token(1),
                whitespace_token("   "),
                hash_comment_token("comment2   "),
                line_token(2),
                hash_comment_token("comment3   "),
                line_token(3),
                line_token(4),
                hash_comment_token("comment4"),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

TEST_CASE("brackets and braces", "[tokenizer]") {
    SECTION("open curlies") {
        string source = "{{";
        token_list expected{
                tokens::start_token(),
                tokens::open_curly_token(),
                tokens::open_curly_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("close curlies") {
        string source = "}}";
        token_list expected{
                tokens::start_token(),
                tokens::close_curly_token(),
                tokens::close_curly_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("open and close curlies") {
        string source = "{}";
        token_list expected{
                tokens::start_token(),
                tokens::open_curly_token(),
                tokens::close_curly_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("open squares") {
        string source = "[[";
        token_list expected{
                tokens::start_token(),
                tokens::open_square_token(),
                tokens::open_square_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("close curlies") {
        string source = "]]";
        token_list expected{
                tokens::start_token(),
                tokens::close_square_token(),
                tokens::close_square_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }

    SECTION("open and close curlies") {
        string source = "[]";
        token_list expected{
                tokens::start_token(),
                tokens::open_square_token(),
                tokens::close_square_token(),
                tokens::end_token()
        };
        tokenizer_test(source, expected);
    }
}

void test_for_config_error(string source) {
    token_iterator iter(fake_origin(), unique_ptr<istringstream>(new istringstream(source)), true);
    while (iter.has_next()) {
        iter.next();
    }
}

TEST_CASE("catch syntax erros", "[tokenizer]") {
    SECTION("nothing after backslash") {
        string source = " \"\\\" ";
        REQUIRE_THROWS(test_for_config_error(source));
    }

    SECTION("\\q is not a valid escape sequence") {
        string source = " \"\\q\" ";
        REQUIRE_THROWS(test_for_config_error(source));
    }

    SECTION("unicode byte sequence missing bytes") {
        string source = " \"\\u012\" ";
        REQUIRE_THROWS(test_for_config_error(source));

        source = " \"\\u01\" ";
        REQUIRE_THROWS(test_for_config_error(source));

        source = " \"\\u1\" ";
        REQUIRE_THROWS(test_for_config_error(source));

        source = " \"\\u\" ";
        REQUIRE_THROWS(test_for_config_error(source));
    }

    SECTION("missing closing quotes") {
        string source = "\"";
        REQUIRE_THROWS(test_for_config_error(source));

        source = "\"abc";
        REQUIRE_THROWS(test_for_config_error(source));
    }

    SECTION("invalid lone characters") {
        string source = "\"\\\"";
        REQUIRE_THROWS(test_for_config_error(source));

        source = "$";
        REQUIRE_THROWS(test_for_config_error(source));

        source = "${";
        REQUIRE_THROWS(test_for_config_error(source));
    }
}
