#include <internal/values/config_double.hpp>

using namespace std;

namespace hocon {

    config_double::config_double(shared_origin origin, double value, string original_text) :
            config_number(move(origin), move(original_text)), _value(value) { }

    std::string config_double::transform_to_string() const {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    unwrapped_value config_double::unwrapped() const {
        return _value;
    }

    int64_t config_double::long_value() const {
        return static_cast<int64_t>(_value);
    }

    double config_double::double_value() const {
        return _value;
    }

    shared_value config_double::new_copy(shared_origin origin) const {
        return make_shared<config_double>(move(origin), _value, _original_text);
    }

}  // namespace hocon
