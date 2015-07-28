#include <internal/config_int.hpp>

using namespace std;

namespace hocon {

    config_int::config_int(simple_config_origin origin, int value, string original_text) :
            config_number(origin, original_text), _value(value) { }

    std::string config_int::transform_to_string() {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    long config_int::long_value() const {
        return _value;
    }

    double config_int::double_value() const {
        return _value;
    }

    config_int* config_int::new_copy(simple_config_origin origin) {
        return new config_int(origin, _value, config_number::transform_to_string());
    }

}  // namespace hocon
