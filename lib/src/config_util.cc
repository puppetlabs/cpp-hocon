#include <internal/config_util.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
        return c <= 0x001F;
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

    string render_string_unquoted_if_possible(string const& s) {
        // this can quote unnecessarily as long as it never fails to quote when
        // necessary
        if (s.empty()) {
            return render_json_string(s);
        }

        // if it starts with a hyphen or number, we have to quote
        // to ensure we end up with a string and not a number
        char first = s[0];
        if (isdigit(first) || first == '-') {
            return render_json_string(s);
        }

        if (boost::starts_with(s, "include") || boost::starts_with(s, "true") || boost::starts_with(s, "false") ||
                boost::starts_with(s, "null") || boost::starts_with(s, "//")) {
            return render_json_string(s);
        }

        // only unquote if it's pure alphanumeric
        for (char c : s) {
            if (!(isalpha(c) || isdigit(c) || c == '-')) {
                return render_json_string(s);
            }
        }

        return s;
    }

    void extract_filename_from_path(const std::string& path,
                                    std::string *file_dir,
                                    std::string *file_name) {
        char sep = '/';
        size_t i = path.rfind(sep, path.length());
        if (string::npos != i) {
            file_dir->assign(path.substr(0, i + 1));
            file_name->assign(path.substr(i + 1, path.length() - i));
        } else {
            file_dir->assign("");
            file_name->assign(path);
        }
    }

    void full_path_operator::stash() {
        _str_stack.push(_current_dir);
        _current_dir.assign("");
    }

    void full_path_operator::append(const std::string& dir) {
        if (! _current_dir.empty() && ! dir.empty() && dir[0] == '/') {
            stash();
        }
        _current_dir.append(dir);
    }

    int full_path_operator::remove(const std::string& dir) {
        size_t cur_dir_len = _current_dir.length();
        size_t dir_len = dir.length();
        if (cur_dir_len < dir_len) {
            return -1;
        } 
        if (_current_dir.substr(cur_dir_len - dir_len, dir_len).compare(dir)) {
            return -2;
        }
        _current_dir.assign(_current_dir.substr(0, cur_dir_len - dir_len));
        if (_current_dir.empty() && ! _str_stack.empty()) {
            _current_dir.assign(_str_stack.top());
            _str_stack.pop();
        }
        return 0;
    }
    
    std::string full_path_operator::extend(const std::string& str) {
        return _current_dir + str;
    }

}  // namespace hocon
