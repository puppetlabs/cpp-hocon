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
#include <internal/path.hpp>

namespace hocon {

    std::shared_ptr<simple_config_origin> fake_origin(std::string description = "fake", int line_number = 0);

    /** Tokens */
    std::shared_ptr<value> string_token(std::string text,
                                        config_string_type type = config_string_type::QUOTED);

    std::shared_ptr<value> bool_token(bool boolean);

    std::shared_ptr<value> double_token(double number, std::string original_text);

    std::shared_ptr<value> int_token(int number, std::string original_text);

    std::shared_ptr<value> null_token();

    std::shared_ptr<substitution> substitution_token(shared_token inner, bool optional);

    std::shared_ptr<line> line_token(int line_number);

    std::shared_ptr<ignored_whitespace> whitespace_token(std::string whitespace);

    std::shared_ptr<unquoted_text> unquoted_text_token(std::string text);

    std::shared_ptr<double_slash_comment> double_slash_comment_token(std::string text);

    std::shared_ptr<hash_comment> hash_comment_token(std::string text);

    /** Nodes */
    std::shared_ptr<config_node_simple_value> colon_node();

    std::shared_ptr<config_node_simple_value> open_brace_node();

    std::shared_ptr<config_node_simple_value> close_brace_node();

    /** Paths */
    path test_path(std::initializer_list<std::string> path_elements);
}  // namespace hocon
