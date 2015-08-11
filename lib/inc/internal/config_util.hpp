#pragma once

namespace hocon {

    bool is_whitespace(char codepoint);

    bool is_whitespace_not_newline(char codepoint);

    bool is_C0_control(char c);

}  // namespace hocon
