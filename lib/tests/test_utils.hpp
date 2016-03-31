#pragma once

#include <internal/simple_config_origin.hpp>
#include <internal/tokens.hpp>
#include <internal/values/config_string.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_double.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_int.hpp>
#include <internal/values/config_null.hpp>
#include <internal/nodes/config_node_simple_value.hpp>

#include <string>
#include <memory>
#include <hocon/path.hpp>
#include <internal/nodes/config_node_field.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/nodes/config_node_comment.hpp>

namespace hocon {

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

    std::shared_ptr<config_int> int_value(int i);

    /** Paths */
    path test_path(std::initializer_list<std::string> path_elements);

    shared_object parse_object(std::string);

    shared_config parse_config(std::string);
}  // namespace hocon
