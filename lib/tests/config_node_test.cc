#include <catch.hpp>

#include "test_utils.hpp"

#include <limits>
#include <internal/path_parser.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/nodes/config_node_array.hpp>
#include <internal/nodes/config_node_concatenation.hpp>

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

/*  This test implementation of abstract_config_node allows
 *  shallow testing of its basic methods.
 */
class concrete_config_node : public abstract_config_node {
public:
    token_list get_tokens() const { return _tokens; };
    token_list _tokens;
};

TEST_CASE("abstract_config_node") {
    concrete_config_node node;
    shared_token token1 = hash_comment_token("token1");
    shared_token token2 = hash_comment_token("token2");

    SECTION("render should return all tokens concatenated together") {
        node._tokens.push_back(token1);
        node._tokens.push_back(token2);
        REQUIRE(node.render() == "#token1#token2");
    }

    SECTION("render should return an empty string when there are no tokens") {
        REQUIRE(node.render() == "");
    }

    SECTION("== operator when objects are equivalent") {
        concrete_config_node another_node;
        node._tokens.push_back(token1);
        another_node._tokens.push_back(token1);
        REQUIRE(node == another_node);
    }

    SECTION("== operator when objects are not equivalent") {
        concrete_config_node another_node;
        node._tokens.push_back(token1);
        another_node._tokens.push_back(token2);
        REQUIRE_FALSE(node == another_node);
    }
}

void single_token_test(shared_token t) {
    config_node_single_token node(t);
    REQUIRE(node.render() == t->token_text());
}

TEST_CASE("single tokens", "[config-node]") {
    SECTION("basic config nodes") {
        single_token_test(tokens::start_token());
        single_token_test(tokens::end_token());
        single_token_test(tokens::open_curly_token());
        single_token_test(tokens::close_curly_token());
        single_token_test(tokens::open_square_token());
        single_token_test(tokens::close_square_token());
        single_token_test(tokens::comma_token());
        single_token_test(tokens::colon_token());
        single_token_test(tokens::plus_equals_token());
        single_token_test(tokens::equals_token());

        single_token_test(unquoted_text_token("foo"));
        single_token_test(whitespace_token("   "));
        single_token_test(line_token(1));
        single_token_test(double_slash_comment_token("slash comment"));
        single_token_test(hash_comment_token("hash comment"));
    }
}

void simple_value_test(shared_token t, config_value::type type) {
    config_node_simple_value node(t);
    // We pass UNSPECIFIED for substitutions, which do not have a config_value::type
    if (type != config_value::type::UNSPECIFIED) {
        REQUIRE(type == node.get_value()->value_type());
    }
    REQUIRE(node.render() == t->token_text());
}

TEST_CASE("simple values", "[config-node]") {
    SECTION("basic value nodes") {
        simple_value_test(int_token(10, "10"), config_value::type::NUMBER);
        simple_value_test(double_token(2.5, "2.5"), config_value::type::NUMBER);
        simple_value_test(bool_token(false), config_value::type::BOOLEAN);
        simple_value_test(bool_token(true), config_value::type::BOOLEAN);
        simple_value_test(null_token(), config_value::type::CONFIG_NULL);
        simple_value_test(string_token("A string."), config_value::type::STRING);
        simple_value_test(unquoted_text_token("unquoted"), config_value::type::STRING);
        simple_value_test(substitution_token(string_token("c.d"), false), config_value::type::UNSPECIFIED);
        simple_value_test(substitution_token(string_token("x.y"), true), config_value::type::UNSPECIFIED);
        simple_value_test(substitution_token(unquoted_text_token("a.b"), false), config_value::type::UNSPECIFIED);
    }
}

TEST_CASE("key nodes", "[config-node]") {
    REQUIRE(node_key("foo")->render() == "foo");
    REQUIRE(node_key("\"Hello how are you today?\"")->render() == "\"Hello how are you today?\"");
}

TEST_CASE("node subpath", "[config-node]") {
    string original_path = "a.b.c.\"@$%@!#$\".\"\".1234.5678";
    auto path_node = node_key(original_path);

    REQUIRE(original_path == path_node->render());
    REQUIRE("c.\"@$%@!#$\".\"\".1234.5678" == path_node->sub_path(2).render());
    REQUIRE("5678" == path_node->sub_path(6).render());
}

void field_node_test(shared_ptr<config_node_path> key, shared_node_value value, shared_node_value new_value) {
    auto key_value_node = node_key_value_pair(key, value);
    REQUIRE((key->render() + " : " + value->render()) == key_value_node->render());
    REQUIRE(key->render() == key_value_node->path()->render());
    REQUIRE(value->render() == key_value_node->get_value()->render());

    auto new_key_value = key_value_node->replace_value(new_value);
    REQUIRE((key->render() + " : " + new_value->render()) == new_key_value->render());
    REQUIRE(new_value->render() == new_key_value->get_value()->render());
}

TEST_CASE("node fields", "[config-node]") {
    SECTION("supports quoted and unquoted keys") {
        field_node_test(node_key("\"abc\""), int_node(123), int_node(345));
        field_node_test(node_key("abc"), int_node(123), int_node(345));
    }

    SECTION("can replace value with values of different types") {
        field_node_test(node_key("\"abc\""), int_node(123), string_node("I am a string"));
        field_node_test(node_key("\"abc\""), int_node(123), make_shared<config_node_object>(
        shared_node_list { open_brace_node(), close_brace_node() }));
    }
}

void top_level_value_replace_test(shared_node_value value,
                                  shared_node_value new_value,
                                  string key = "foo")
{
    shared_node_list children { open_brace_node(),
                                node_key_value_pair(node_key(key), value),
                                close_brace_node() };
    config_node_object complex { children };
    auto new_node = complex.set_value_on_path(key, new_value);
    string original_text = "{" + key + " : " + value->render() + "}";
    string final_text = "{" + key + " : " + new_value->render() + "}";

    REQUIRE(original_text == complex.render());
    REQUIRE(final_text == new_node->render());
}

TEST_CASE("replace nodes", "[config-node]") {
    SECTION("simple values can be replaced") {
        top_level_value_replace_test(int_node(10), int_node(15));
        top_level_value_replace_test(long_node(numeric_limits<int>::max() + 1ll), int_node(1));
        top_level_value_replace_test(double_node(3.14), int_node(2));
        top_level_value_replace_test(bool_node(false), bool_node(false));
        top_level_value_replace_test(bool_node(true), null_node());
        top_level_value_replace_test(null_node(), string_node("I am a string"));
        top_level_value_replace_test(string_node("string here!"), unquoted_text_node("thisisunquoted"));
        top_level_value_replace_test(unquoted_text_node("unquotedtext"), substitution_node(string_token("c.d"), false));
        top_level_value_replace_test(int_node(10), substitution_node(unquoted_text_token("x.y"), true));
        top_level_value_replace_test(int_node(10), substitution_node(unquoted_text_token("a.b"), false));
        top_level_value_replace_test(substitution_node(unquoted_text_token("c.d"), false), int_node(10));
    }

    auto node_array = make_shared<config_node_array>(shared_node_list {
            open_brace_node(),
            int_node(10),
            space_node(),
            comma_node(),
            space_node(),
            int_node(15),
            close_brace_node()
    });

    SECTION("arrays can be replaced") {
        top_level_value_replace_test(int_node(10), node_array);
        top_level_value_replace_test(node_array, int_node(10));
        top_level_value_replace_test(node_array, make_shared<config_node_object>(shared_node_list {
                open_brace_node(),
                close_brace_node()
        }));
    }

    auto nested_map = make_shared<config_node_object>(
            shared_node_list { open_brace_node(),
                               node_key_value_pair(node_key("abc"), string_node("a string")),
                               close_brace_node()
            });

    SECTION("objects can be replaced") {
        top_level_value_replace_test(nested_map, int_node(10));
        top_level_value_replace_test(int_node(10), nested_map);
        top_level_value_replace_test(node_array, nested_map);
        top_level_value_replace_test(nested_map, node_array);
        top_level_value_replace_test(nested_map,
                                     make_shared<config_node_object>(shared_node_list { open_brace_node(),
                                                                                        close_brace_node() }));
    }

    SECTION("concatenations can be replaced") {
        auto concat = make_shared<config_node_concatenation>(shared_node_list {
                int_node(10),
                space_node(),
                string_node("hello")
        });

        top_level_value_replace_test(concat, int_node(10));
        top_level_value_replace_test(int_node(12), concat);
        top_level_value_replace_test(nested_map, concat);
        top_level_value_replace_test(concat, nested_map);
        top_level_value_replace_test(concat, node_array);
        top_level_value_replace_test(node_array, concat);
    }

    SECTION("a.b key format") {
        top_level_value_replace_test(int_node(10), nested_map, "foo.bar");
    }
}

void remove_duplicates_test(shared_node_value value1, shared_node_value value2, shared_node_value value3) {
    auto key = node_key("foo");
    auto key_val1 = node_key_value_pair(key, value1);
    auto key_val2 = node_key_value_pair(key, value2);
    auto key_val3 = node_key_value_pair(key, value3);

    auto complex = make_shared<config_node_object>(shared_node_list { key_val1, key_val2, key_val3 });
    string original_text = key_val1->render() + key_val2->render() + key_val3->render();
    string final_text = key->render() + " : 15";

    REQUIRE(original_text == complex->render());
    REQUIRE(final_text == complex->set_value_on_path("foo", int_node(15))->render());
}

TEST_CASE("duplicates of a key are removed from a map", "[config-node]") {
    auto empty_map_node = make_shared<config_node_object>(shared_node_list { open_brace_node(), close_brace_node() });
    auto empty_array = make_shared<config_node_array>(shared_node_list { open_brace_node(), close_brace_node() });

    remove_duplicates_test(int_node(10), bool_node(true), null_node());
    remove_duplicates_test(empty_map_node, empty_map_node, empty_map_node);
    remove_duplicates_test(empty_array, empty_array, empty_array);
    remove_duplicates_test(int_node(10), empty_map_node, empty_array);
}

void nonexistent_path_test(shared_node_value value) {
    auto node = make_shared<config_node_object>(
            shared_node_list { node_key_value_pair(node_key("bar"), int_node(15)) });
    REQUIRE("bar : 15" == node->render());
    auto new_node = node->set_value_on_path("foo", value);
    string final_text = "bar : 15, foo : " + value->render();
    REQUIRE(final_text == new_node->render());
}

TEST_CASE("creates non-existent paths", "[config-node]") {
    nonexistent_path_test(int_node(10));
    nonexistent_path_test(make_shared<config_node_object>(
            shared_node_list { open_brace_node(),
                               node_key_value_pair(node_key("foo"), double_node(3.14)),
                               close_brace_node() }));
    nonexistent_path_test(make_shared<config_node_array>(shared_node_list {
            open_brace_node(),
            int_node(15),
            close_brace_node()
    }));
}

TEST_CASE("replace nested nodes") {
    string orig_text = "foo : bar\nbaz : {\n\t\"abc.def\" : 123\n\t"
                       "//this is a comment about the below setting"
                       "\n\n\tabc : {\n\t\tdef : \"this is a string\""
                       "\n\t\tghi : ${\"a.b\"}\n\t}\n}\nbaz.abc.ghi "
                       ": 52\nbaz.abc.ghi : 53\n}";
    auto lowest_level_map = make_shared<config_node_object>(shared_node_list {
          open_brace_node(),
          line_node(6),
          whitespace_node("\t\t"),
          node_key_value_pair(node_key("def"), string_node("\"this is a string\"")),
          line_node(7),
          whitespace_node("\t\t"),
          node_key_value_pair(node_key("ghi"), substitution_node(string_token("\"a.b\""), false)),
          line_node(8),
          whitespace_node("\t"),
          close_brace_node()
    });
    auto higher_level_map = make_shared<config_node_object>(shared_node_list {
            open_brace_node(),
            line_node(2),
            whitespace_node("\t"),
            node_key_value_pair(node_key("\"abc.def\""), int_node(123)),
            line_node(3),
            whitespace_node("\t"),
            double_slash_comment_node("this is a comment about the below setting"),
            line_node(4),
            line_node(5),
            whitespace_node("\t"),
            node_key_value_pair(node_key("abc"), lowest_level_map),
            line_node(9),
            close_brace_node()
    });
    auto original_node = make_shared<config_node_object>(shared_node_list {
            node_key_value_pair(node_key("foo"), unquoted_text_node("bar")),
            line_node(1),
            node_key_value_pair(node_key("baz"), higher_level_map),
            line_node(10),
            node_key_value_pair(node_key("baz.abc.ghi"), int_node(52)),
            line_node(11),
            node_key_value_pair(node_key("baz.abc.ghi"), int_node(53)),
            line_node(12),
            close_brace_node()
    });
    REQUIRE(orig_text == original_node->render());

    string final_text = "foo : bar\nbaz : {\n\t\"abc.def\" : true\n\t"
            "//this is a comment about the below setting"
            "\n\n\tabc : {\n\t\tdef : false\n\t\t\n\t\t\"this.does.not.exist@@@+$#\""
            " : {\n\t\t  end : doesnotexist\n\t\t}\n\t}\n}\n\nbaz.abc.ghi : randomunquotedString\n}";

    // Paths with quotes in the name are treated as a single path rather than multiple subpaths
    auto new_node = original_node->set_value_on_path("baz.\"abc.def\"", bool_node(true));
    new_node = new_node->set_value_on_path("baz.abc.def", bool_node(false));

    // Repeats are removed from nested maps
    new_node = new_node->set_value_on_path("baz.abc.ghi", unquoted_text_node("randomunquotedString"));

    // Missing paths are added to the top level if they don't appear anywhere, including in nested maps
    new_node = new_node->set_value_on_path("baz.abc.\"this.does.not.exist@@@+$#\".end",
                                           unquoted_text_node("doesnotexist"));

    REQUIRE(final_text == new_node->render());
}

