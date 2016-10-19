#include <hocon/config_value_factory.hpp>
#include <internal/values/config_null.hpp>
#include <internal/values/config_string.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_double.hpp>
#include <internal/values/config_int.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/simple_config_list.hpp>
#include <internal/values/simple_config_object.hpp>

namespace hocon {

    using namespace std;

    class config_value_visitor : public boost::static_visitor<shared_value> {
    public:
        // TODO: If use cases of from_any_ref require other types to produce config_nulls,
        // we can revise this behavior
        shared_value operator()(boost::blank null_value) const {
            return make_shared<config_null>(nullptr);
        }

        shared_value operator()(string str) const {
            return make_shared<config_string>(nullptr, str, config_string_type::QUOTED);
        }

        shared_value operator()(int64_t num) const {
            return make_shared<config_long>(nullptr, num, "");
        }

        shared_value operator()(double num) const {
            return make_shared<config_double>(nullptr, num, "");
        }

        shared_value operator()(int num) const {
            return make_shared<config_int>(nullptr, num, "");
        }

        shared_value operator()(bool boolean) const {
            return make_shared<config_boolean>(nullptr, boolean);
        }

        shared_value operator()(vector<unwrapped_value> value_list) const {
            vector<shared_value> config_values;
            for (unwrapped_value v : value_list) {
                config_values.emplace_back(boost::apply_visitor(config_value_visitor(), v));
            }
            return make_shared<simple_config_list>(nullptr, config_values);
        }

        shared_value operator()(unordered_map<string, unwrapped_value> value_map) const {
            unordered_map<string, shared_value> config_map;
            for (auto pair : value_map) {
                config_map[pair.first] = boost::apply_visitor(config_value_visitor(), pair.second);
            }
            return make_shared<simple_config_object>(nullptr, config_map);
        }
    };

    shared_value config_value_factory::from_any_ref(unwrapped_value value, std::string origin) {
        // If no origin is specified, create a default one.
        if (origin.empty()) {
            origin = "hardcoded value";
        }
        auto conf_origin = make_shared<simple_config_origin>(origin);
        return boost::apply_visitor(config_value_visitor(), value)->with_origin(conf_origin);
    }
}  // namespace hocon
