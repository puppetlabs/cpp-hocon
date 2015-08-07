#include <catch.hpp>

#include "test_utils.hpp"

#include <internal/nodes/abstract_config_node.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/nodes/config_node_simple_value.hpp>

using namespace std;
using namespace hocon;

/*  This test implementation of abstract_config_node allows
 *  shallow testing of its basic methods.
 */
class concrete_config_node : public abstract_config_node {
public:
    vector<shared_ptr<token>> get_tokens() const { return _tokens; };
    vector<shared_ptr<token>> _tokens;
};

TEST_CASE("abstract_config_node") {
    concrete_config_node node;
    shared_ptr<token> token1 = hash_comment_token("token1");
    shared_ptr<token> token2 = hash_comment_token("token2");

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

void single_token_test(shared_ptr<token> t) {
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

void simple_value_test(shared_ptr<token> t) {
    config_node_simple_value node(t);
    REQUIRE(node.render() == t->token_text());
}

TEST_CASE("simple values", "[config-node]") {
    SECTION("basic value nodes") {
        simple_value_test(int_token(10, "10"));
        simple_value_test(double_token(2.5, "2.5"));
        simple_value_test(bool_token(false));
        simple_value_test(bool_token(true));
        simple_value_test(null_token());
        simple_value_test(string_token("A string."));
        simple_value_test(unquoted_text_token("unquoted"));
        simple_value_test(substitution_token(string_token("c.d"), false));
        simple_value_test(substitution_token(string_token("x.y"), true));
        simple_value_test(substitution_token(unquoted_text_token("a.b"), false));
    }
}