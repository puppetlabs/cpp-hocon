#include <internal/values/config_string.hpp>

using namespace std;

namespace hocon {

    config_string::config_string(shared_origin origin, string text, config_string_type quoted) :
        config_value(move(origin)), _text(move(text)), _quoted(quoted) { }

    config_value_type config_string::value_type() const {
        return config_value_type::STRING;
    }

    string config_string::transform_to_string() const {
        return _text;
    }

    bool config_string::was_quoted() const {
        return _quoted == config_string_type::QUOTED;
    }

}  // namespace hocon
