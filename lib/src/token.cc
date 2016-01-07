#include <internal/token.hpp>
#include <hocon/config_origin.hpp>

using namespace std;

namespace hocon {

    unsupported_exception::unsupported_exception(string const& message) :
        runtime_error(message) { }

    token::token(token_type type, config_origin origin, string token_text, string debug_string) :
        _token_type(type), _origin(move(origin)), _token_text(move(token_text)),
        _debug_string(move(debug_string)) { }

    token::token(token_type type, string token_text, string debug_string) :
        token(type, config_origin(), move(token_text), move(debug_string)) {}

    token_type token::get_token_type() const {
        return _token_type;
    }

    string token::token_text() const {
        return _token_text;
    }

    string token::to_string() const {
        return _debug_string.empty() ? _token_text : _debug_string;
    }

    int token::line_number() const {
        if (_origin) {
            return _origin.line_number();
        } else {
            return -1;
        }
    }

    config_origin const& token::origin() const {
        if (_origin) {
            return _origin;
        } else {
            throw unsupported_exception("This token has no origin.");
        }
    }

    bool token::operator==(const token& other) const {
        return get_token_type() == other.get_token_type();
    }

}  // namespace hocon
