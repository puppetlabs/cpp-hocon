#include <internal/tokenizer.hpp>
#include <internal/config_util.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_null.hpp>
#include <internal/values/config_double.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_string.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    problem_exception::problem_exception(problem prob) :
            runtime_error(prob.message()), _problem(move(prob)) { }

    problem const& problem_exception::get_problem() const {
        return _problem;
    }

    /** Whitespace Saver */
    token_iterator::whitespace_saver::whitespace_saver() : _last_token_was_simple_value(false) { }

    void token_iterator::whitespace_saver::add(char c) {
        _whitespace += c;
    }

    shared_token token_iterator::whitespace_saver::check(token_type type, shared_origin base_origin, int line_number)
    {
        if (is_simple_value(type)) {
            return next_is_simple_value(base_origin, line_number);
        } else {
            return next_is_not_simple_value(base_origin, line_number);
        }
    }

    /**
     * Called if the next token is not a simple value;
     * discards any whitespace we were saving between
     * simple values.
     */
    shared_token token_iterator::whitespace_saver::next_is_not_simple_value(shared_origin base_origin, int line_number)
    {
        _last_token_was_simple_value = false;
        return create_whitespace_token(base_origin, line_number);
    }

    /**
     * Called if the next token IS a simple value,
     * so creates a whitespace token if the previous
     * token also was simple.
     */
    shared_token token_iterator::whitespace_saver::next_is_simple_value(shared_origin base_origin, int line_number) {
        shared_token t = create_whitespace_token(base_origin, line_number);
        if (!_last_token_was_simple_value) {
            _last_token_was_simple_value = true;
        }
        return t;
    }

    shared_token token_iterator::whitespace_saver::create_whitespace_token(shared_origin base_origin, int line_number) {
        if (_whitespace.length() > 0) {
            shared_token t;
            if (_last_token_was_simple_value) {
                t = make_shared<unquoted_text>(line_origin(base_origin, line_number), _whitespace);
            } else {
                t = make_shared<ignored_whitespace>(line_origin(base_origin, line_number), _whitespace);
            }
            _whitespace = "";  // reset
            return t;
        }
        return nullptr;
    }

    /**
     * Token Iterator
     */
    token_iterator::token_iterator(shared_origin origin, unique_ptr<std::istream> input, bool allow_comments) :
            _origin(move(origin)), _input(move(input)), _allow_comments(allow_comments),
            _line_number(1), _line_origin(_origin->with_line_number(1))
    {
        _tokens.push(tokens::start_token());
    }

    token_iterator::token_iterator(shared_origin origin, unique_ptr<std::istream> input, config_syntax flavor) :
        token_iterator(move(origin), move(input), flavor != config_syntax::JSON) {}

    bool token_iterator::start_of_comment(char c) {
        if (!*_input) {
            return false;
        } else {
            if (_allow_comments) {
                if (c == '#') {
                    return true;
                } else if (c == '/') {
                    char maybe_second_slash = _input->peek();
                    return maybe_second_slash == '/';  // Double slash indicates a comment
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    char token_iterator::next_char_after_whitespace(whitespace_saver& saver) {
        char c = 0;
        while (*_input) {
            c = _input->get();
            if (is_whitespace_not_newline(c)) {
                saver.add(c);
                continue;
            } else {
                return c;
            }
        }
        return c;
    }

    bool token_iterator::is_simple_value(token_type type) {
        return type == token_type::SUBSTITUTION ||
                type == token_type::VALUE ||
                type == token_type::UNQUOTED_TEXT;
    }

    shared_origin token_iterator::line_origin(shared_origin origin, int line_number) {
        return origin->with_line_number(line_number);
    }

    string token_iterator::render(token_list tokens) {
        string rendered_text = "";
        for (auto&& t : tokens) {
            rendered_text += t->token_text();
        }
        return rendered_text;
    }

    shared_token token_iterator::pull_comment(char first_char) {
        bool double_slash = false;
        if (first_char == '/') {
            int discard = _input->get();
            if (discard != '/') {
                throw config_exception(_("called pull_comment() but // not seen"));
            }
            double_slash = true;
        }

        string result;
        int c = _input->get();
        while (*_input && c != '\n') {
            result += c;
            c = _input->get();
        }
        if (c == '\n') {
            _input->putback(c);
        }
        if (double_slash) {
            return make_shared<double_slash_comment>(_line_origin, result);
        } else {
            return make_shared<hash_comment>(_line_origin, result);
        }
    }

    /** Characters JSON allows a number to start with */
    static string first_number_chars() {
        static const string first_number_chars_ = "0123456789-";
        return first_number_chars_;
    }

    /** Characters allowed in a JSON number */
    static string number_chars() {
        static const string number_chars_ = "0123456789eE+-.";
        return number_chars_;
    }

    /** Character that stop an unquoted string */
    static const string not_in_unquoted_text = "$\"{}[]:=,+#`^?!@*&\\";

    /**
     * The rules here are intended to maximize convenience while
     * avoiding confusion with real valid JSON. Basically anything
     * that parses as JSON is treated the JSON way and otherwise
     * we assume it's a string and let the parser sort it out.
     */
    shared_token token_iterator::pull_unquoted_text() {
        auto const& origin = _line_origin;
        string result;
        char c = _input->get();
        while (*_input
               && not_in_unquoted_text.find(c) == string::npos
               && !is_whitespace(c)
               && !start_of_comment(c)) {
            result += c;

            // We parse true/false/null tokens as such no matter
            // what is after them, as long as they are at the
            // start of the unquoted token.
            if (result.length() == 4) {
                if (result == "true") {
                    return make_shared<value>(make_shared<config_boolean>(origin, true));
                } else if (result == "null") {
                    return make_shared<value>(make_shared<config_null>(origin));
                }
            } else if (result.length() == 5) {
                if (result == "false") {
                    return make_shared<value>(make_shared<config_boolean>(origin, false));
                }
            }

            c = _input->get();
        }

        // Put back the char that ended the unquoted text
        _input->putback(c);


        return make_shared<unquoted_text>(origin, result);
    }

    shared_token token_iterator::pull_number(char first_char) {
        string result;
        result += first_char;
        bool contained_decimal_or_E = false;
        char c = _input->get();
        while (*_input && number_chars().find(c) != string::npos) {
            if (c == '.' || c == 'e' || c == 'E') {
                contained_decimal_or_E = true;
            }
            result += c;
            c = _input->get();
        }

        // The last char we looked at wasn't part of the number, put it back
        _input->putback(c);

        try {
            if (contained_decimal_or_E) {
                return make_shared<value>(config_number::new_number(
                        _line_origin, boost::lexical_cast<double>(result), result));
            } else {
                return make_shared<value>(config_number::new_number(
                        _line_origin, boost::lexical_cast<int64_t>(result), result));
            }
        } catch (boost::bad_lexical_cast const& ex) {
            // not a number after all, see if it's an unquoted string
            for (char character : result) {
                if (not_in_unquoted_text.find(character) != string::npos) {
                    throw config_exception(_("Line {1}: Reserved character '{2}' not allowed outside quotes", std::to_string(_line_number), character));
                }
            }
            // no disallowed chars, so we decide this was a string and not a number
            return make_shared<unquoted_text>(_line_origin, result);
        }
    }

    void token_iterator::pull_escape_sequence(string& parsed, string& original) {
        if (!*_input) {
            throw config_exception(_("End of input but backslash in string had nothing after it"));
        }

        // This is needed so we return the unescaped escape characters back out when rendering
        // the token
        char escaped = _input->get();
        original += "\\";
        original += escaped;

        switch (escaped) {
            case '"':
                parsed += '"';
                break;
            case '\\':
                parsed += '\\';
                break;
            case '/':
                parsed += '/';
                break;
            case 'b':
                parsed += '\b';
                break;
            case 'f':
                parsed += '\f';
                break;
            case 'n':
                parsed += '\n';
                break;
            case 'r':
                parsed += '\r';
                break;
            case 't':
                parsed += '\t';
                break;
            case 'u': {
                char utf[5] = {};
                for (int i = 0; i < 4; i++) {
                    if (!*_input) {
                        throw config_exception(_("End of input but expecting 4 hex digits for \\uXXXX escape"));
                    }
                    utf[i] = _input->get();
                }
                original += string(utf);
                short character;
                sscanf(utf, "%hx", &character);
                wchar_t buffer[] { static_cast<wchar_t>(character), '\0'};
                parsed += boost::nowide::narrow(buffer);
            }
                break;
            default:
                throw config_exception(_("backslash followed by {1}, this is not a valid escape sequence. (Quoted strings use JSON escaping, so use double-backslash \\\\ for literal backslash)", string(1, escaped)));
        }
    }

    void token_iterator::append_triple_quoted_string(string& parsed, string& original) {
        // We are after the opening triple quote and need to consume the close triple
        int consecutive_quotes = 0;
        while (true) {
            char c = _input->get();
            if (c == '"') {
                consecutive_quotes++;
            } else if (consecutive_quotes >= 3) {
                // The last three quotes end the string, and the others are kept.
                parsed = parsed.substr(0, parsed.length() - 3);
                _input->putback(c);
                break;
            } else {
                consecutive_quotes = 0;
                if (!*_input) {
                    throw config_exception(_("End of input but triple-quoted string was still open"));
                } else if (c == '\n') {
                    _line_number++;
                    _line_origin = _origin->with_line_number(_line_number);
                }
            }
            parsed += c;
            original += c;
        }
    }

    shared_token token_iterator::pull_quoted_string() {
        string result;

        // We need a second string builder to keep track of escape characters.
        // We want to return them exactly as they appeared in the original text,
        // which means we will need a new StringBuilder to escape escape characters
        // so we can also keep the actual value of the string. This is gross.
        string original = "\"";

        while (true) {
            if (!*_input) {
                throw config_exception(_("End of input but string quote was still open"));
            }

            char c = _input->get();
            if (c == '\\') {
                pull_escape_sequence(result, original);
            } else if (c == '"') {
                original += '"';
                break;
            } else if (is_C0_control(c)) {
                throw config_exception(_("Line {1}: JSON does not allow unescaped {2} in quoted strings, use a backslash escape", std::to_string(_line_number), string(1, c)));
            } else {
                result += c;
                original += c;
            }
        }

        // maybe switch to triple quoted string
        if (result.length() == 0) {
            char third = _input->get();
            if (third == '"') {
                original += third;
                append_triple_quoted_string(result, original);
            } else {
                _input->putback(third);
            }
        }

        return make_shared<value>(make_shared<config_string>(_line_origin, result, config_string_type::QUOTED),
                                               original);
    }

    shared_token const& token_iterator::pull_plus_equals() {
        char c = _input->get();
        if (c != '=') {
            throw config_exception(_("'+' not followed by '=', '{1}' not allowed after '+'", string(1, c)));
        }
        return tokens::plus_equals_token();
    }

    shared_token token_iterator::pull_substitution() {
        // The initial '$' has already been consumed
        auto const& origin = _line_origin;
        char c = _input->get();
        if (c != '{') {
            throw config_exception(_("'$' not followed by '{', '{1}' not allowed after '$'", string(1, c)));
        }

        bool optional = false;
        c = _input->get();
        if (c == '?') {
            optional = true;
        } else {
            _input->putback(c);
        }

        whitespace_saver saver;
        token_list expression;

        shared_token t;
        do {
            t = pull_next_token(saver);

            // note that we avoid validating the allowed tokens inside
            // the substitution here; we even allow nested substitutions
            // in the tokenizer. The parser sorts it out.
            if (t == tokens::close_curly_token()) {
                // we found the end of the substitution
                break;
            } else if (t == tokens::end_token()) {
                throw config_exception(_("Substitution '${' was not closed with a '}'"));
            } else {
                shared_token whitespace = saver.check(t->get_token_type(), origin, _line_number);
                if (whitespace != nullptr) {
                    expression.push_back(whitespace);
                }
                expression.push_back(t);
            }
        } while (true);

        return make_shared<substitution>(_line_origin, optional, expression);
    }

    shared_token token_iterator::pull_next_token(whitespace_saver& saver) {
        char c = next_char_after_whitespace(saver);
        if (!*_input) {
            return tokens::end_token();
        } else if (c == '\n') {
            shared_token newline = make_shared<line>(_line_origin);
            _line_number++;
            _line_origin = _origin->with_line_number(_line_number);
            return newline;
        } else {
            shared_token t;
            if (start_of_comment(c)) {
                t = pull_comment(c);
            } else {
                switch (c) {
                    case '"':
                        t = pull_quoted_string();
                        break;
                    case '$':
                        t = pull_substitution();
                        break;
                    case ':':
                        t = tokens::colon_token();
                        break;
                    case ',':
                        t = tokens::comma_token();
                        break;
                    case '=':
                        t = tokens::equals_token();
                        break;
                    case '{':
                        t = tokens::open_curly_token();
                        break;
                    case '}':
                        t = tokens::close_curly_token();
                        break;
                    case '[':
                        t = tokens::open_square_token();
                        break;
                    case ']':
                        t = tokens::close_square_token();
                        break;
                    case '+':
                        t = pull_plus_equals();
                        break;
                    default:
                        t = nullptr;
                        break;
                }

                if (t == nullptr) {
                    if (first_number_chars().find(c) != string::npos) {
                        t = pull_number(c);
                    } else if (not_in_unquoted_text.find(c) != string::npos) {
                        throw config_exception(_("Reserved character '{1}' is not allowed outside quotes", string(1, c)));
                    } else {
                        _input->putback(c);
                        t = pull_unquoted_text();
                    }
                }
            }

            if (t == nullptr) {
                throw config_exception(_("Failed to generate next token"));
            }

            return t;
        }
    }

    void token_iterator::queue_next_token() {
        shared_token t = pull_next_token(_whitespace_saver);
        shared_token whitespace = _whitespace_saver.check(t->get_token_type(), _origin, _line_number);
        if (whitespace != nullptr) {
           _tokens.push(whitespace);
        }
        _tokens.push(t);
    }

    bool token_iterator::has_next() {
        return !_tokens.empty();
    }

    shared_token token_iterator::next() {
        shared_token t = _tokens.front();
        _tokens.pop();
        if (_tokens.empty() && t != tokens::end_token()) {
            try {
                queue_next_token();
            } catch (config_exception& ex) {
                throw ex;
                // TODO: they add the problem token to the queue here,
                // maybe should switch to problem exceptions after all
                // This will require rewriting some tests, will wait and
                // see how exceptions are handled elsewhere before switching
            }
            if (_tokens.empty()) {
                throw config_exception(_("Tokens queue should not be empty here"));
            }
        }
        return t;
    }

    /** Single token iterator */
    single_token_iterator::single_token_iterator(shared_token token) : _token(move(token)), _has_next(true) { }

    bool single_token_iterator::has_next() {
        return _has_next;
    }

    shared_token single_token_iterator::next() {
        if (_has_next) {
            _has_next = false;
            return _token;
        }
        return nullptr;
    }

    /** Token list iterator */
    token_list_iterator::token_list_iterator(token_list tokens) : _tokens(move(tokens)), _index(-1) { }

    bool token_list_iterator::has_next() {
        return _index + 1 < static_cast<int>(_tokens.size());
    }

    shared_token token_list_iterator::next() {
        _index++;
        return _tokens[_index];
    }

}  // namespace hocon
