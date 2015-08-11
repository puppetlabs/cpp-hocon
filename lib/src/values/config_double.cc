#include <internal/values/config_double.hpp>

using namespace std;

namespace hocon {

    config_double::config_double(shared_ptr<simple_config_origin> origin,
                                 double value, string original_text) :
            config_number(move(origin), move(original_text)), _value(value) { }

    std::string config_double::transform_to_string() const {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    int64_t config_double::long_value() const {
        return static_cast<int64_t>(_value);
    }

    double config_double::double_value() const {
        return _value;
    }

}  // namespace hocon
