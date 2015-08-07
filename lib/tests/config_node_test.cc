#include <catch.hpp>

#include "test_utils.hpp"

#include <internal/nodes/abstract_config_node.hpp>

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