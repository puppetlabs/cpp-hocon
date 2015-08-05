#include <internal/abstract_config_value.hpp>

using namespace std;

namespace hocon {

    abstract_config_value::abstract_config_value(simple_config_origin origin) :
        _origin(move(origin))
    {
    }

    string abstract_config_value::transform_to_string() const {
        return "";
    }

}  // namespace hocon
