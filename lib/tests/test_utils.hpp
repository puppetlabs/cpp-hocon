#pragma once

#include <internal/simple_config_origin.hpp>
#include <internal/substitution_expression.hpp>
#include <internal/tokens.hpp>
#include <internal/values/config_string.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_double.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_int.hpp>
#include <internal/values/config_null.hpp>
#include <internal/values/config_reference.hpp>
#include <internal/values/config_concatenation.hpp>
#include <internal/nodes/config_node_simple_value.hpp>

#include <string>
#include <memory>
#include <vector>
#include <hocon/path.hpp>
#include <internal/nodes/config_node_field.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/nodes/config_node_comment.hpp>

#define REQUIRE_STRING_CONTAINS(str, match) REQUIRE(std::string(str).find(match) != std::string::npos)
#define REQUIRE_STRING_NOT_CONTAINS(str, match) REQUIRE(std::string(str).find(match) == std::string::npos)

namespace hocon { namespace test_utils {

    shared_origin fake_origin(std::string description = "fake", int line_number = 0);

    /** Tokens */
    std::shared_ptr<value> string_token(std::string text, config_string_type type = config_string_type::QUOTED);

    std::shared_ptr<value> bool_token(bool boolean);

    std::shared_ptr<value> double_token(double number, std::string original_text);

    std::shared_ptr<value> int_token(int number, std::string original_text);

    std::shared_ptr<value> long_token(int64_t number, std::string original_text);

    std::shared_ptr<value> null_token();

    std::shared_ptr<substitution> substitution_token(shared_token inner, bool optional = false);

    std::shared_ptr<line> line_token(int line_number);

    std::shared_ptr<ignored_whitespace> whitespace_token(std::string whitespace);

    std::shared_ptr<unquoted_text> unquoted_text_token(std::string text);

    std::shared_ptr<double_slash_comment> double_slash_comment_token(std::string text);

    std::shared_ptr<hash_comment> hash_comment_token(std::string text);

    /** Nodes */
    std::shared_ptr<config_node_single_token> colon_node();

    std::shared_ptr<config_node_single_token> open_brace_node();

    std::shared_ptr<config_node_single_token> close_brace_node();

    std::shared_ptr<config_node_single_token> space_node();

    std::shared_ptr<config_node_single_token> comma_node();

    std::shared_ptr<config_node_field> node_key_value_pair(std::shared_ptr<config_node_path> key,
                                                           shared_node_value value);

    std::shared_ptr<config_node_path> node_key(std::string key);

    std::shared_ptr<config_node_simple_value> int_node(int number);

    std::shared_ptr<config_node_simple_value> long_node(int64_t number);

    std::shared_ptr<config_node_simple_value> double_node(double number);

    std::shared_ptr<config_node_simple_value> string_node(std::string text);

    std::shared_ptr<config_node_simple_value> null_node();

    std::shared_ptr<config_node_simple_value> bool_node(bool value);

    std::shared_ptr<config_node_simple_value> unquoted_text_node(std::string text);

    std::shared_ptr<config_node_simple_value> substitution_node(shared_token key, bool optional);

    std::shared_ptr<config_node_single_token> line_node(int line_number);

    std::shared_ptr<config_node_single_token> whitespace_node(std::string whitespace);

    std::shared_ptr<config_node_comment> double_slash_comment_node(std::string text);

    // it's important that these do NOT use the public API to create the
    // instances, because we may be testing that the public API returns the
    // right instance by comparing to these, so using public API here would
    // make the test compare public API to itself.
    std::shared_ptr<config_int> int_value(int i);
    // TODO: remaining value helpers

    std::shared_ptr<config_boolean> bool_value(bool b);

    std::shared_ptr<config_null> null_value();

    std::shared_ptr<config_string> string_value(std::string s);

    std::shared_ptr<config_double> double_value(double d);

    std::shared_ptr<config_reference> subst(std::string ref, bool optional = false);

    std::shared_ptr<config_concatenation> subst_in_string(std::string ref, bool optional = false);

    /** Paths */
    path test_path(std::initializer_list<std::string> path_elements);

    shared_object parse_object(std::string);

    shared_config parse_config(std::string);

    struct parse_test {
        parse_test(std::string t, bool lbe = false, bool wm = false)
            : test(move(t)), lift_behavior_unexpected(lbe), whitespace_matters(wm) { }

        std::string test;
        bool lift_behavior_unexpected, whitespace_matters;
    };

    std::vector<parse_test> const& invalid_json();

    std::vector<parse_test> const& invalid_conf();

    std::vector<parse_test> const& valid_json();

    std::vector<parse_test> const& valid_conf();

    std::vector<parse_test> whitespace_variations(std::vector<parse_test> const& tests, bool valid_in_lift);

    std::string fixture_path(std::string fixture_name);
}}  // namespace hocon::test_utils
