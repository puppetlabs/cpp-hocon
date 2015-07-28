#include <internal/config_boolean.hpp>

using namespace std;

namespace hocon {

    config_boolean::config_boolean(simple_config_origin origin, bool value) :
        abstract_config_value(origin), _value(value) { }

    config_value_type config_boolean::value_type() {
        return config_value_type::BOOLEAN;
    }

    string config_boolean::transform_to_string(){
        return _value ? "true" : "false";
    }

    config_boolean* config_boolean::new_copy(simple_config_origin origin) {
        return new config_boolean(origin, _value);
    }

}  // namespace hocon
