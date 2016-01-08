#include <hocon/config_value.hpp>
#include <internal/exception.hpp>
#include <internal/exception.hpp>
#include <internal/config_util.hpp>
#include <hocon/config_object.hpp>
#include <internal/objects/simple_config_object.hpp>
#include <internal/simple_config_origin.hpp>

using namespace std;

namespace hocon {

    config_value::config_value(shared_origin origin) :
        _origin(move(origin)) { }

    string config_value::transform_to_string() const {
        return "";
    }

    shared_origin const& config_value::origin() const {
        return _origin;
    }

    resolve_status config_value::get_resolve_status() const {
        return resolve_status::RESOLVED;
    }

    string config_value::render() const {
        return render(config_render_options());
    }

    string config_value::render(config_render_options options) const {
        string result;
        render(result, 0, true, "", options);
        return result;
    }

    resolve_status resolve_status_from_values(std::vector<shared_value> const& values) {
        for (auto& v: values) {
            if (v->get_resolve_status() == resolve_status::UNRESOLVED) {
                return resolve_status::UNRESOLVED;
            }
        }
        return resolve_status::RESOLVED;
    }

    void config_value::render(std::string &result, int indent, bool at_root, std::string at_key,
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

    void config_value::render(std::string &result, int indent, bool at_root,
                                       config_render_options options) const {
        result += transform_to_string();
    }

    shared_config config_value::at_path(shared_origin origin, path raw_path) const {
        path parent = raw_path.parent();
        shared_config result = at_key(origin, *raw_path.last());
        while (!parent.empty()) {
            string key = *parent.last();
            result = result->at_key(origin, key);
            parent = parent.parent();
        }
        return result;
    }

    shared_config config_value::at_key(shared_origin origin, std::string const& key) const {
        unordered_map<string, shared_value> map { make_pair(key, shared_from_this()) };
        return simple_config_object(origin, map).to_config();
    }

    shared_config config_value::at_key(std::string const& key) const {
        return at_key(make_shared<simple_config_origin>("at_key(" + key + ")"), key);
    }

    shared_config config_value::at_path(std::string const& path_expression) const {
        shared_origin origin = make_shared<simple_config_origin>("at_path(" + path_expression + ")");
        return at_path(move(origin), path::new_path(path_expression));
    }

    config_value::no_exceptions_modifier::no_exceptions_modifier(string prefix): _prefix(std::move(prefix)) {}

    shared_value config_value::no_exceptions_modifier::modify_child_may_throw(string key_or_null, shared_value v) const {
        try {
            return modify_child(key_or_null, v);
        } catch (runtime_error& e) {
            throw e;
        } catch (exception& e) {
            throw config_exception("Unexpected exception:", e);
        }
    }

    shared_value config_value::no_exceptions_modifier::modify_child(string key, shared_value v) const {
        return v->relativized(_prefix);
    }

}  // namespace hocon
