#include <internal/config_double.hpp>

using namespace std;

namespace hocon {

    config_double::config_double(simple_config_origin origin, double value, string original_text) :
            config_number(origin, original_text), _value(value) { }

    std::string config_double::transform_to_string() {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    long config_double::long_value() const {
        return (long)_value;
    }

    double config_double::double_value() const {
        return _value;
    }

    config_double* config_double::new_copy(simple_config_origin origin) {
        return new config_double(origin, _value, config_number::transform_to_string());
    }

}  // namespace hocon
