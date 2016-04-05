#include <catch.hpp>
#include <hocon/parser/config_document_factory.hpp>
#include <internal/simple_config_document.hpp>
#include "fixtures.hpp"

using namespace hocon;
using namespace std;

void replace_test(string original_text, string final_text, string new_value,
                  string replace_path, config_syntax syntax) {
    auto config_doc = config_document_factory::parse_string(original_text,
                                                            config_parse_options().set_syntax(syntax));
    REQUIRE(original_text == config_doc->render());
    auto new_doc = config_doc->with_value_text(replace_path, new_value);
    REQUIRE(final_text == new_doc->render());
}

TEST_CASE("replacement of elements in a document", "[config-doc]") {
    SECTION("replacements in a simple map") {
        replace_test("{\"a\":1}", "{\"a\":2}", "2", "a", config_syntax::JSON);
        replace_test("{\"a\":1}", "{\"a\":2}", "2", "a", config_syntax::CONF);
    }

    SECTION("replacement in a map without braces") {
        replace_test("a: b\nc = d", "a: b\nc = 12", "12", "c", config_syntax::CONF);
    }

    string original_text = "{\n\"a\":123,\n\"b\": 123.456,\n\"c\": true,\n\"d\": false,\n\"e\": null,\n"
        "\"f\": \"a string\",\n\"g\": [1,2,3,4,5],\n\"h\": {\n\t\"a\": 123,\n\t\"b\": {\n\t\t\"a\": 12\n\t},\n"
        "\"c\": [1, 2, 3, {\"a\": \"b\"}, [1,2,3]]\n}\n}";
    SECTION("replacements in a complex map") {
        string final_text = "{\n\"a\":123,\n\"b\": 123.456,\n\"c\": true,\n\"d\": false,\n\"e\": null,\n"
                "\"f\": \"a string\",\n\"g\": [1,2,3,4,5],\n\"h\": {\n\t\"a\": 123,\n\t\"b\": {\n\t\t\"a\": "
                "\"i am now a string\"\n\t},\n\"c\": [1, 2, 3, {\"a\": \"b\"}, [1,2,3]]\n}\n}";
        replace_test(original_text, final_text, "\"i am now a string\"", "h.b.a", config_syntax::JSON);
        replace_test(original_text, final_text, "\"i am now a string\"", "h.b.a", config_syntax::CONF);
    }

    SECTION("replace a value with a map") {
        string final_text = "{\n\"a\":123,\n\"b\": 123.456,\n\"c\": true,\n\"d\": false,\n\"e\": null,\n"
                "\"f\": \"a string\",\n\"g\": [1,2,3,4,5],\n\"h\": {\n\t\"a\": 123,\n\t\"b\": {\n\t\t\"a\": "
                "{\"a\":\"b\", \"c\":\"d\"}\n\t},\n\"c\": [1, 2, 3, {\"a\": \"b\"}, [1,2,3]]\n}\n}";
        replace_test(original_text, final_text, "{\"a\":\"b\", \"c\":\"d\"}", "h.b.a", config_syntax::JSON);
        replace_test(original_text, final_text, "{\"a\":\"b\", \"c\":\"d\"}", "h.b.a", config_syntax::CONF);
    }

    SECTION("replace a value with an array") {
        string final_text = "{\n\"a\":123,\n\"b\": 123.456,\n\"c\": true,\n\"d\": false,\n\"e\": null,\n"
                "\"f\": \"a string\",\n\"g\": [1,2,3,4,5],\n\"h\": {\n\t\"a\": 123,\n\t\"b\": {\n\t\t\"a\": "
                "[1,2,3,4,5]\n\t},\n\"c\": [1, 2, 3, {\"a\": \"b\"}, [1,2,3]]\n}\n}";
        replace_test(original_text, final_text, "[1,2,3,4,5]", "h.b.a", config_syntax::JSON);
        replace_test(original_text, final_text, "[1,2,3,4,5]", "h.b.a", config_syntax::CONF);
    }

    SECTION("replace a value with a concatenation") {
        string final_text = "{\n\"a\":123,\n\"b\": 123.456,\n\"c\": true,\n\"d\": false,\n\"e\": null,\n"
                "\"f\": \"a string\",\n\"g\": [1,2,3,4,5],\n\"h\": {\n\t\"a\": 123,\n\t\"b\": {\n\t\t\"a\": "
                "this is a concatenation 123 456 {a:b} [1,2,3] {a: this is another 123 concatenation null true}\n\t},\n"
                "\"c\": [1, 2, 3, {\"a\": \"b\"}, [1,2,3]]\n}\n}";
        replace_test(original_text, final_text, "this is a concatenation 123 456 {a:b} [1,2,3] "
                "{a: this is another 123 concatenation null true}", "h.b.a", config_syntax::CONF);
    }
}

TEST_CASE("removal of duplicates", "[config-doc]") {
    string original_text = "{a: b, a.b.c: d, a: e}";
    auto config_doc = config_document_factory::parse_string(original_text);
    REQUIRE("{a: 2}" == config_doc->with_value_text("a", "2")->render());

    original_text = "{a: b, a: e, a.b.c:d}";
    config_doc = config_document_factory::parse_string(original_text);
    REQUIRE("{a: 2, }" == config_doc->with_value_text("a", "2")->render());

    original_text = "{a.b.c: d}";
    config_doc = config_document_factory::parse_string(original_text);
    REQUIRE("{ a : 2}" == config_doc->with_value_text("a", "2")->render());
}

TEST_CASE("set new value", "[config-doc]") {
    SECTION("braced root node") {
        string original_text = "{\n\t\"a\":\"b\",\n\t\"c\":\"d\"\n}";
        string final_text_conf = "{\n\t\"a\":\"b\",\n\t\"c\":\"d\"\n\t\"e\" : \"f\"\n}";
        string final_text_json = "{\n\t\"a\":\"b\",\n\t\"c\":\"d\",\n\t\"e\" : \"f\"\n}";
        replace_test(original_text, final_text_json, "\"f\"", "\"e\"", config_syntax::JSON);
        replace_test(original_text, final_text_conf, "\"f\"", "\"e\"", config_syntax::CONF);
    }

    SECTION("no brace at root node") {
        string original_text = "\"a\":\"b\",\n\"c\":\"d\"\n";
        string final_text = "\"a\":\"b\",\n\"c\":\"d\"\n\"e\" : \"f\"\n";
        replace_test(original_text, final_text, "\"f\"", "\"e\"", config_syntax::CONF);
    }

    SECTION("multi-level") {
        string original_text = "a:b\nc:d";
        string final_text = "a:b\nc:d\ne : {\n  f : {\n    g : 12\n  }\n}";
        replace_test(original_text, final_text, "12", "e.f.g", config_syntax::CONF);

        original_text = "{\"a\":\"b\",\n\"c\":\"d\"}";
        final_text = "{\"a\":\"b\",\n\"c\":\"d\",\n  \"e\" : {\n    \"f\" : {\n      \"g\" : 12\n    }\n  }}";
        replace_test(original_text, final_text, "12", "e.f.g", config_syntax::JSON);
    }
}

TEST_CASE("parse from file", "[doc-parser]") {
    string original_text = "{ a : b }";
    string directory = TEST_FILE_DIR;
    auto doc = config_document_factory::parse_file(directory + "/test.conf");
    REQUIRE(doc->render() == original_text);
}