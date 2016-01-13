#include <internal/values/config_null.hpp>

using namespace std;

namespace hocon {

    config_null::config_null(shared_origin origin) :
            config_value(move(origin)) { }

    config_value_type config_null::value_type() const {
        return config_value_type::CONFIG_NULL;
    }

    string config_null::transform_to_string() const {
        return "null";
    }

    shared_value config_null::new_copy(shared_origin origin) const {
        return make_shared<config_null>(move(origin));
    }

}  // namespace hocon
