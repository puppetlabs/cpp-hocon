#include <internal/values/config_boolean.hpp>

using namespace std;

namespace hocon {

    config_boolean::config_boolean(shared_origin origin, bool value) :
        config_value(move(origin)), _value(value) { }

    config_value_type config_boolean::value_type() const {
        return config_value_type::BOOLEAN;
    }

    string config_boolean::transform_to_string() const {
        return _value ? "true" : "false";
    }

    bool config_boolean::bool_value() const {
        return _value;
    }

}  // namespace hocon
