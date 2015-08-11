#include <internal/config_util.hpp>
#include <cctype>

namespace hocon {

    bool is_whitespace(char c) {
        switch (c) {
            case ' ':
            case '\n':
            // TODO: these complain about char overflow, what do.
//            case u'\u00A0':
//            case u'\u2007':
//            case u'\u202F':
//            case u'\uFEFF':
                return true;
            default:
                return isspace(c);
        }
    }

    bool is_whitespace_not_newline(char c) {
        return c != '\n' && is_whitespace(c);
    }

    bool is_C0_control(char c) {
        return c >= 0 && c <= 0x001F;
    }

}  // namespace hocon
