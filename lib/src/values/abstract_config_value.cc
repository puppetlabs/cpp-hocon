#include <internal/values/abstract_config_value.hpp>
#include <internal/config_util.hpp>
#include <hocon/config_object.hpp>

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

    string abstract_config_value::render() const {
        return render(config_render_options());
    }

    string abstract_config_value::render(config_render_options options) const {
        string result;
        render(result, 0, true, "", options);
        return result;
    }

    void abstract_config_value::render(std::string &result, int indent, bool at_root, std::string at_key,
                                         config_render_options options) const {
        if (!at_key.empty()) {
            string rendered_key;
            if (options.get_json()) {
                rendered_key = render_json_string(at_key);
            } else {
                rendered_key = render_string_unquoted_if_possible(at_key);
            }

            result += rendered_key;
            if (options.get_json()) {
                result += options.get_formatted() ? " : " : ":";
            } else {
                // in non-JSON we can omit the color or equals before an object
                if (dynamic_cast<const config_object*>(this)) {
                    if (options.get_formatted()) {
                        result += " ";
                    }
                } else {
                    result += "=";
                }
            }
        }
        render(result, indent, at_root, options);
    }

    void abstract_config_value::render(std::string &result, int indent, bool at_root,
                                       config_render_options options) const {
        result += transform_to_string();
    }

}  // namespace hocon
