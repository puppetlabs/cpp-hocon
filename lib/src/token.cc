#include <internal/token.hpp>
#include <internal/simple_config_origin.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    unsupported_exception::unsupported_exception(string const& message) :
        runtime_error(message) { }

    token::token(token_type type, shared_origin origin, string token_text, string debug_string) :
        _token_type(type), _origin(move(origin)), _token_text(move(token_text)),
        _debug_string(move(debug_string)) { }


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
            return _origin->line_number();
        } else {
            return -1;
        }
    }

    shared_origin const& token::origin() const {
        if (_origin) {
            return _origin;
        } else {
            throw unsupported_exception(_("This token has no origin."));
        }
    }

    bool token::operator==(const token& other) const {
        return get_token_type() == other.get_token_type();
    }

}  // namespace hocon
