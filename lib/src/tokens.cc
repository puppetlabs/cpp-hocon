#include <internal/tokens.hpp>
#include <internal/config_exception.hpp>
#include <internal/tokenizer.hpp>
#include <iostream>

using namespace std;

namespace hocon {

    /** Value token */
    value::value(shared_value value) :
            token(token_type::VALUE, value->transform_to_string()), _value(move(value)) { }

    value::value(shared_value value, string original_text) :
            token(token_type::VALUE, original_text),
            _value(move(value)) { }

    std::string value::to_string() const {
        return _value->render();
    }

    config_origin const& value::origin() const {
        return _value->origin();
    }

    shared_value value::get_value() const {
        return _value;
    }

    bool value::operator==(const token& other) const {
        return other.get_token_type() == token_type::VALUE &&
               other.to_string() == to_string();
    }

    /** Line token */
    line::line(config_origin origin) :
            token(token_type::NEWLINE, move(origin), "\n") { }

    string line::to_string() const {
        return "'\\n'@" + std::to_string(line_number());
    }

    bool line::operator==(const token& other) const {
        return (other.get_token_type() == token_type::NEWLINE) && (line_number() == other.line_number());
    }

    /** Unquoted text token */
    unquoted_text::unquoted_text(config_origin origin, string text) :
            token(token_type::UNQUOTED_TEXT, move(origin), move(text)) { }

    string unquoted_text::to_string() const {
        return "'" + token_text() + "' (UNQUOTED)";
    }

    bool unquoted_text::operator==(const token& other) const {
        return (other.get_token_type() == token_type::UNQUOTED_TEXT) &&
                (other.token_text() == token_text());
    }

    /** Ignored whitespace token */
    ignored_whitespace::ignored_whitespace(config_origin origin, string whitespace) :
        token(token_type::IGNORED_WHITESPACE, move(origin), move(whitespace)) { }

    ignored_whitespace::ignored_whitespace(string whitespace) :
        ignored_whitespace(config_origin(), move(whitespace)) { }

    string ignored_whitespace::to_string() const {
        return "'" + token_text() + "' (WHITESPACE)";
    }

    bool ignored_whitespace::operator==(const token& other) const {
        return other.get_token_type() == token_type::IGNORED_WHITESPACE &&
                other.token_text() == token_text();
    }

    /** Problem token */
    problem::problem(config_origin origin, string what, string message,
        bool suggest_quotes) : token(token_type::PROBLEM, move(origin)), _what(move(what)),
        _message(move(message)), _suggest_quotes(suggest_quotes) { }

    string problem::what() const {
        return _what;
    }

    string problem::message() const {
        return _message;
    }

    bool problem::suggest_quotes() const {
        return _suggest_quotes;
    }

    std::string problem::to_string() const {
        return "'" + _what + "' (" + _message + ")";
    }

    bool problem::operator==(const token& other) const {
        try {
            problem const& other_problem = dynamic_cast<problem const&>(other);
            return other_problem.what() == what() &&
                   other_problem.message() == message() &&
                   other_problem.suggest_quotes() == suggest_quotes();
        } catch (bad_cast ex){
            return false;
        }
    }

    /** Comment token */
    comment::comment(config_origin origin, string text) :
        token(token_type::COMMENT, move(origin)), _text(move(text)) { }

    string comment::text() const {
        return _text;
    }

    string comment::to_string() const {
        return "'#" + _text + "' (COMMENT)";
    }

    bool comment::operator==(const token& other) const {
        return other.get_token_type() == token_type::COMMENT && to_string() == other.to_string();
    }

    /** Double-slash comment token */
    double_slash_comment::double_slash_comment(config_origin origin, string text) :
        comment(move(origin), move(text)) { }

    string double_slash_comment::token_text() const {
        return "//" + text();
    }

    /** Hash comment token */
    hash_comment::hash_comment(config_origin origin, string text) :
        comment(move(origin), move(text)) { }

    string hash_comment::token_text() const {
        return "#" + text();
    }

    /** Substitution token */
    substitution::substitution(config_origin origin, bool optional,
        token_list expression) : token(token_type::SUBSTITUTION, move(origin)), _optional(optional),
        _expression(move(expression)) { }

    bool substitution::optional() const {
        return _optional;
    }

    token_list const& substitution::expression() const {
        return _expression;
    }

    string substitution::token_text() const {
        return "${" + string(optional() ? "?" : "") + token_iterator::render(_expression) + "}";
    }

    string substitution::to_string() const {
        string result;
        for (auto&& t : _expression) {
            result += t->token_text();
        }
        return "'${" + result + "}'";
    }

    bool substitution::operator==(const token& other) const {
        return other.get_token_type() == token_type::SUBSTITUTION &&
               to_string() == other.to_string();
    }

    /** Singleton tokens */
    shared_token const& tokens::start_token() {
        static shared_token _start = make_shared<token>(
                token_type::START, "", "start of file");
        return _start;
    }

    shared_token const& tokens::end_token() {
        static shared_token _end = make_shared<token>(
                token_type::END, "", "end of file");
        return _end;
    }

    shared_token const& tokens::comma_token() {
        static shared_token _comma = make_shared<token>(
                token_type::COMMA, ",", "','");
        return _comma;
    }

    shared_token const& tokens::equals_token() {
        static shared_token _equals = make_shared<token>(
                token_type::EQUALS, "=", "'='");
        return _equals;
    }

    shared_token const& tokens::colon_token() {
        static shared_token _colon = make_shared<token>(
                token_type::COLON, ":", "':'");
        return _colon;
    }

    shared_token const& tokens::open_curly_token() {
        static shared_token _open_curly = make_shared<token>(
                token_type::OPEN_CURLY, "{", "'{'");
        return _open_curly;
    }

    shared_token const& tokens::close_curly_token() {
        static shared_token _close_curly = make_shared<token>(
                token_type::CLOSE_CURLY, "}", "'}'");
        return _close_curly;
    }

    shared_token const& tokens::open_square_token() {
        static shared_token _open_square = make_shared<token>(
                token_type::OPEN_SQUARE, "[", "'['");
        return _open_square;
    }

    shared_token const& tokens::close_square_token() {
        static shared_token _close_square = make_shared<token>(
                token_type::CLOSE_SQUARE, "]", "']'");
        return _close_square;
    }

    shared_token const& tokens::plus_equals_token() {
        static shared_token _plus_equals = make_shared<token>(
                token_type::PLUS_EQUALS, "+=", "'+='");
        return _plus_equals;
    }

    /** Static token handler methods */
    shared_value tokens::get_value(shared_token t) {
        if (auto value_token = dynamic_pointer_cast<const value>(t)) {
            return value_token->get_value();
        } else {
            throw config_exception("Tried to get the value of a non-value token.");
        }
    }

    bool tokens::is_value_with_type(shared_token token, config_value_type type) {
        auto value_token = dynamic_pointer_cast<const value>(token);
        if (value_token) {
            return value_token->get_value()->value_type() == type;
        }
        return false;
    }

}  // namespace hocon
