#include <internal/values/config_boolean.hpp>

using namespace std;

namespace hocon {

    config_boolean::config_boolean(shared_origin origin, bool value) :
        abstract_config_value(move(origin)), _value(value) { }

    config_value_type config_boolean::value_type() const {
        return config_value_type::BOOLEAN;
    }

    string config_boolean::transform_to_string() const {
        return _value ? "true" : "false";
    }

}  // namespace hocon
