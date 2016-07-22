#pragma once

#include <string>

namespace hocon {

    bool is_whitespace(signed char codepoint);

    bool is_whitespace_not_newline(signed char codepoint);

    bool is_C0_control(signed char c);

    std::string render_json_string(std::string const& s);

    std::string render_string_unquoted_if_possible(std::string const& s);

}  // namespace hocon
