#include <hocon/config_value.hpp>
#include <internal/simple_config_list.hpp>

using namespace std;

namespace hocon {

    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value)
            : config_list(move(origin)), _value(move(value)), _resolved(resolve_status_from_values(_value)) { }


    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value,
                                           resolve_status status) : simple_config_list(move(origin), move(value)){

        if (status != _resolved) {
            throw config_exception("simple_config_list created with wrong resolve status");
        }
    }
}  // namespace hocon
