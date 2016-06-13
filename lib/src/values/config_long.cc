#include <internal/values/config_long.hpp>

using namespace std;

namespace hocon {

    config_long::config_long(shared_origin origin, int64_t value, string original_text) :
            config_number(move(origin), move(original_text)), _value(value) { }

    std::string config_long::transform_to_string() const {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    unwrapped_value config_long::unwrapped() const {
        return _value;
    }

    int64_t config_long::long_value() const {
        return _value;
    }

    double config_long::double_value() const {
        return _value;
    }

    shared_value config_long::new_copy(shared_origin origin) const {
        return make_shared<config_long>(move(origin), _value, _original_text);
    }

}  // namespace hocon
