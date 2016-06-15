#include <internal/values/config_null.hpp>
#include <boost/blank.hpp>

using namespace std;

namespace hocon {

    config_null::config_null(shared_origin origin) :
            config_value(move(origin)) { }

    config_value::type config_null::value_type() const {
        return config_value::type::CONFIG_NULL;
    }

    string config_null::transform_to_string() const {
        return "null";
    }

    shared_value config_null::new_copy(shared_origin origin) const {
        return make_shared<config_null>(move(origin));
    }

    unwrapped_value config_null::unwrapped() const {
        return boost::blank();
    }

    bool config_null::operator==(config_value const& other) const {
        return dynamic_cast<config_null const*>(&other);
    }

    void config_null::render(string& result, int indent, bool at_root, config_render_options options) const {
        result += "null";
    }

}  // namespace hocon
