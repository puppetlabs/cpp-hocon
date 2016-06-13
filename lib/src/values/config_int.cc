#include <internal/values/config_int.hpp>

using namespace std;

namespace hocon {

    config_int::config_int(shared_origin origin, int value, string original_text) :
            config_number(move(origin), move(original_text)), _value(value) { }

    std::string config_int::transform_to_string() const {
        string s = config_number::transform_to_string();
        if (s.empty()) {
            return to_string(_value);
        } else {
            return s;
        }
    }

    unwrapped_value config_int::unwrapped() const {
        return _value;
    }

    int64_t config_int::long_value() const {
        return _value;
    }

    double config_int::double_value() const {
        return _value;
    }

    shared_value config_int::new_copy(shared_origin origin) const {
        return make_shared<config_int>(move(origin), _value, _original_text);
    }

}  // namespace hocon
