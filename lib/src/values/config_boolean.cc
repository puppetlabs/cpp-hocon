#include <internal/values/config_boolean.hpp>

using namespace std;

namespace hocon {

    config_boolean::config_boolean(shared_origin origin, bool value) :
        config_value(move(origin)), _value(value) { }

    config_value::type config_boolean::value_type() const {
        return config_value::type::BOOLEAN;
    }

    string config_boolean::transform_to_string() const {
        return _value ? "true" : "false";
    }

    bool config_boolean::bool_value() const {
        return _value;
    }

    unwrapped_value config_boolean::unwrapped() const {
        return _value;
    }

    shared_value config_boolean::new_copy(shared_origin origin) const {
        return make_shared<config_boolean>(move(origin), _value);
    }

    bool config_boolean::operator==(config_value const& other) const {
        return equals<config_boolean>(other, [&](config_boolean const& o) { return _value == o._value; });
    }

}  // namespace hocon
