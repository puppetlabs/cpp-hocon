#include <internal/values/abstract_config_value.hpp>

using namespace std;

namespace hocon {

    abstract_config_value::abstract_config_value(shared_origin origin) :
        _origin(move(origin)) { }

    string abstract_config_value::transform_to_string() const {
        return "";
    }

    shared_origin const& abstract_config_value::origin() const {
        return _origin;
    }

    resolved_status abstract_config_value::resolved() const {
        return resolved_status::RESOLVED;
    }

}  // namespace hocon
