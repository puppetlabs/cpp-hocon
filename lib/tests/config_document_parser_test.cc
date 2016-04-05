#include <catch.hpp>
#include <internal/config_document_parser.hpp>
#include <hocon/parser/config_document_factory.hpp>

#include "test_utils.hpp"

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

shared_ptr<config_node_root> conf_parse(string original_text) {
    return config_document_parser::parse(
            token_iterator(fake_origin(),
                           unique_ptr<istringstream>(new istringstream(original_text)),
                           true),
            fake_origin(),
            config_parse_options());
}

shared_ptr<config_node_root> json_parse(string original_text) {
    return config_document_parser::parse(
            token_iterator(fake_origin(),
                           unique_ptr<istringstream>(new istringstream(original_text)),
                           false),
            fake_origin(),
            config_parse_options().set_syntax(config_syntax::JSON));
}

shared_node_value parse_value_conf(string original_text) {
    return config_document_parser::parse_value(
            token_iterator(fake_origin(),
                           unique_ptr<istringstream>(new istringstream(original_text)),
                           true),
            fake_origin(),
            config_parse_options());
}

shared_node_value parse_value_json(string original_text) {
    return config_document_parser::parse_value(
            token_iterator(fake_origin(),
                           unique_ptr<istringstream>(new istringstream(original_text)),
                           false),
            fake_origin(),
            config_parse_options().set_syntax(config_syntax::JSON));
}

void parse_values_test(string original_text, bool simple = true, string final_text = "") {
    string expected = final_text.empty() ? original_text : final_text;
    auto node = parse_value_conf(original_text);
    REQUIRE(expected == node->render());
    if (simple) {
        REQUIRE(dynamic_pointer_cast<const config_node_simple_value>(node));
    } else {
        REQUIRE(dynamic_pointer_cast<const config_node_complex_value>(node));
    }

    auto json_node = parse_value_json(original_text);
    REQUIRE(expected == json_node->render());
    if (simple) {
        REQUIRE(dynamic_pointer_cast<const config_node_simple_value>(node));
    } else {
        REQUIRE(dynamic_pointer_cast<const config_node_complex_value>(node));
    }
}

TEST_CASE("parse values", "[doc-parser]") {
    SECTION("parse simple values") {
        parse_values_test("123");
        parse_values_test("123.456");
        parse_values_test("\"a string\"");
        parse_values_test("true");
        parse_values_test("false");
        parse_values_test("null");
    }

    SECTION("parse complex values") {
        parse_values_test("{\"a\": \"b\"}", false);
        parse_values_test("[\"a\", \"b\", \"c\"]", false);
    }

    SECTION("parse concatenations in CONF mode") {
        string original_text = "123 456 \"abc\"";
        auto node = parse_value_conf(original_text);
        REQUIRE(original_text == node->render());
    }

    SECTION("parse keys with no separators and object values in CONF mode") {
        string original_text = "{\"foo\" { \"bar\" : 12 } }";
        auto node = parse_value_conf(original_text);
        REQUIRE(original_text == node->render());
    }
}

void invalid_json_test(string original_text, string message) {
    try {
        json_parse(original_text);
    } catch (parse_exception& ex) {
        REQUIRE(string(ex.what()).find(message) != string::npos);
    }
}

TEST_CASE("parse single value errors", "[doc-parser]") {
    SECTION("illegal leading and trailing whitespace and comments") {
        REQUIRE_THROWS(parse_value_conf("   123"));
        REQUIRE_THROWS(parse_value_conf("123   "));
        REQUIRE_THROWS(parse_value_conf(" 123 "));
        REQUIRE_THROWS(parse_value_conf("\n123"));
        REQUIRE_THROWS(parse_value_conf("123\n"));
        REQUIRE_THROWS(parse_value_conf("\n123\n"));
        REQUIRE_THROWS(parse_value_conf("#this is a comment\n123#comment"));
    }

    SECTION("illegal whitespace after concatenation") {
        REQUIRE_THROWS(parse_value_conf("123 456 789   "));
    }
}

TEST_CASE("invalid JSON parse errors", "[doc-parser]") {
    invalid_json_test("unquotedtext", "Token not allowed in valid JSON");

    SECTION("no substitutions in JSON") {
        invalid_json_test("${a.b}", "Substitutions (${} syntax) not allowed in JSON");
    }

    SECTION("no concatenations in JSON") {
        invalid_json_test("{ \"foo\": 123 456 789 } ", "Expecting close brace '}' or a comma");
    }

    SECTION("value separators required in JSON") {
        invalid_json_test("{\"foo\" { \"bar\" : 12 } }", "may not be followed by token: '{'");
    }

    SECTION("require root element in JSON") {
        invalid_json_test("\"a\": 123, \"b\": 456", "Document must have an object or array at root");
    }
}

TEST_CASE("parse empty document", "[doc-parser]") {
    auto node = conf_parse("");
    REQUIRE(dynamic_pointer_cast<const config_node_object>(node->value()));
    REQUIRE(node->value()->children().empty());

    auto node2 = conf_parse("#comment\n#comment\n\n");
    REQUIRE(dynamic_pointer_cast<const config_node_object>(node2->value()));
}

void parse_test_string(string original_text) {
    auto node = conf_parse(original_text);
    REQUIRE(original_text == node->render());
}

TEST_CASE("comprehensive parse test") {
    SECTION("without curly braces") {
        parse_test_string("foo:bar");
        parse_test_string(" foo : bar ");
        parse_test_string("include \"foo.conf\" ");
        parse_test_string("   \nfoo:bar\n   ");
        parse_test_string("aUnquoted: bar\naString = \"qux\"\naNumb:123\naDouble=123.456\naTrue=true\naFalse=false\n"
                          "aNull=null\naSub =  ${a.b}\ninclude \"foo.conf\"");
    }

    SECTION("with curly braces") {
        parse_test_string("{}");
        parse_test_string("{foo:bar}");
        parse_test_string("{  foo  :  bar  }");
        parse_test_string("{foo:bar}    ");
        parse_test_string("{include \"foo.conf\"}");
        parse_test_string("  \n{foo:bar}\n  ");
        parse_test_string("{\naUnquoted: bar\naString = \"qux\"\naNumb:123\naDouble=123.456\naTrue=true\naFalse=false\n"
                          "aNull=null\naSub =  ${a.b}\ninclude \"foo.conf\"\n}");
    }

    SECTION("nested maps") {
        parse_test_string("\nfoo.bar.baz : {\n\tqux : \"abcdefg\"\n\t\"abc\".def.\"ghi\" : 123\n\tabc = "
                          "{ food:bar }\n}\nqux = 123.456\n");
    }

    SECTION("comments in maps") {
        parse_test_string("{\nfoo: bar\n// this is a comment\nbaz:qux // this is another comment\n}");
    }

    SECTION("arrays") {
        parse_test_string("[]");
        parse_test_string("[foo]");
        parse_test_string("[foo,]");
        parse_test_string("[foo,]   ");
        parse_test_string("   \n[]\n   ");
        parse_test_string("[foo, bar,\"qux\", 123,123.456, true,false, null, ${a.b}]");
        parse_test_string("[foo,   bar,\"qux\"   , 123  123.456, true,false, null,   ${a.b}   ]");
    }

    SECTION("basic concatenation") {
        parse_test_string("[foo bar baz qux]");
        parse_test_string("{foo: foo bar baz qux}");
        parse_test_string("[abc 123 123.456 null true false [1, 2, 3] {a:b}, 2]");
    }

    SECTION("all together now") {
        parse_test_string("{\nfoo: bar baz   qux    ernie\n// The above was a concatenation\n\nbaz  =  [ abc 123, {a:12\n"
        "\t\t\t\tb: {\n\t\t\t\t\tc: 13\n\t\t\t\t\td: {\n\t\t\t\t\t\ta: 22\n\t\t\t\t\t\tb: \"abcdefg\" # this is a "
        "comment\n\t\t\t\t\t\tc: [1, 2, 3]\n\t\t\t\t\t}\n\t\t\t\t}\n\t\t\t\t}, # this was an object in an array\n"
        "\t\t\t\t//The above value is a map containing a map containing a map, all in an array\n\t\t\t\t22,\n"
        "\t\t\t\t// The below is an array contained in another array\n\t\t\t\t[1,2,3]]\n//This is a map with some "
        "nested maps and array within it, as well as as some concatenations\nqux {\n\tbaz: abc 123\n\tbar: {\n\t\t"
        "baz: abcdefg\n\t\tbar: {\n\t\t\ta: null\n\t\t\tb: true\n\t\t\tc: [true false 123, null, [1, 2, 3]]\n\t\t}"
        "\n\t}\n}\n// Did I cover everything?\n}");
    }
}
