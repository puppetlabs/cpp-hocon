#include <internal/values/config_null.hpp>

using namespace std;

namespace hocon {

    config_null::config_null(config_origin origin) :
            config_value(move(origin)) { }

    config_value_type config_null::value_type() const {
        return config_value_type::CONFIG_NULL;
    }

    string config_null::transform_to_string() const {
        return "null";
    }

}  // namespace hocon
