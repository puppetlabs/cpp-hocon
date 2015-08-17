#include <internal/config_util.hpp>

using namespace std;

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

    string render_json_string(string const& s) {
        string result = "\"";
        for (char c : s) {
            switch (c) {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\b':
                    result += "\\b";
                    break;
                case '\f':
                    result += "\\f";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    // TODO: The java has a case here for is_C0_control() that converts c to
                    // a unicode literal (\uXXXX). Not sure how to handle at present.
                    result += c;
            }
        }
        result += "\"";
        return result;
    }

}  // namespace hocon
