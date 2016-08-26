#include <hocon/config_value.hpp>
#include <internal/config_util.hpp>
#include <hocon/config_object.hpp>
#include <hocon/config_exception.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/container.hpp>
#include <internal/values/config_delayed_merge.hpp>
#include <internal/resolve_result.hpp>
#include <leatherman/locale/locale.hpp>
#include <algorithm>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

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
        for (auto& v : values) {
            if (v->get_resolve_status() == resolve_status::UNRESOLVED) {
                return resolve_status::UNRESOLVED;
            }
        }
        return resolve_status::RESOLVED;
    }

    void config_value::render(std::string &result, int indent, bool at_root, std::string const& at_key, config_render_options options) const {
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
                // in non-JSON we can omit the colon or equals before an object
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

    void config_value::indent(std::string &result, int indent, config_render_options const& options) {
        if (options.get_formatted()) {
            result.append(indent*4, ' ');
        }
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

    shared_value config_value::with_origin(shared_origin origin) const {
        if (_origin == origin) {
            return shared_from_this();
        } else {
            return new_copy(move(origin));
        }
    }

    resolve_result<shared_value>
    config_value::resolve_substitutions(resolve_context const& context,
                                        resolve_source const& source) const
    {
        return resolve_result<shared_value>(context, shared_from_this());
    }

    std::vector<shared_value> config_value::replace_child_in_list(std::vector<shared_value> const& values,
                                                                  shared_value const& child, shared_value replacement)
    {
        vector<shared_value> new_list = values;
        auto it = find(new_list.begin(), new_list.end(), child);
        assert(it != values.end());

        if (replacement) {
            *it = move(replacement);
        } else {
            new_list.erase(it);
        }
        return new_list;
    }

    bool config_value::has_descendant_in_list(std::vector<shared_value> const& values, shared_value const& descendant)
    {
        // Simple breadth-first-search for descendants
        // We look for the same object via pointer comparison, not equivalent objects.
        if (find(values.begin(), values.end(), descendant) != values.end()) {
            return true;
        }

        for (auto& v : values) {
            if (auto c = dynamic_pointer_cast<const container>(v)) {
                if (c->has_descendant(descendant)) {
                    return true;
                }
            }
        }

        return false;
    }

    config_value::no_exceptions_modifier::no_exceptions_modifier(string prefix): _prefix(std::move(prefix)) {}

    shared_value config_value::no_exceptions_modifier::modify_child_may_throw(string key_or_null, shared_value v) {
        try {
            return modify_child(key_or_null, v);
        } catch (runtime_error& e) {
            throw e;
        } catch (exception& e) {
            throw config_exception(_("Unexpected exception:{1}", e.what()));
        }
    }

    shared_value config_value::no_exceptions_modifier::modify_child(string key, shared_value v) const {
        return v->relativized(_prefix);
    }

    shared_ptr<const config_mergeable> config_value::with_fallback(std::shared_ptr<const config_mergeable> mergeable) const {
        if (ignores_fallbacks()) {
            return shared_from_this();
        } else {
            auto other = dynamic_pointer_cast<const config_mergeable>(mergeable)->to_fallback_value();

            if (auto unmergeable_other = dynamic_pointer_cast<const unmergeable>(other)) {
                return merged_with_the_unmergeable(unmergeable_other);
            } else if (auto object_other = dynamic_pointer_cast<const config_object>(other)) {
                return merged_with_object(object_other);
            } else {
                return merged_with_non_object(dynamic_pointer_cast<const config_value>(other));
            }
        }
    }

    void config_value::require_not_ignoring_fallbacks() const {
        if (ignores_fallbacks()) {
            throw config_exception(_("method should not have been called with ignores_fallbacks=true"));
        }
    }

    bool config_value::ignores_fallbacks() const {
        return get_resolve_status() == resolve_status::RESOLVED;
    }

    shared_value config_value::with_fallbacks_ignored() const {
        if (ignores_fallbacks()) {
            return shared_from_this();
        } else {
            throw config_exception(_("value class doesn't implement forced fallback-ignoring"));
        }
    }

    shared_value config_value::construct_delayed_merge(shared_origin origin, std::vector<shared_value> stack) const {
        return make_shared<config_delayed_merge>(move(origin), move(stack));
    }

    shared_value config_value::merged_with_the_unmergeable(std::vector<shared_value> stack,
                                                           std::shared_ptr<const unmergeable> fallback) const {
        require_not_ignoring_fallbacks();

        // if we turn out to be an object, and the fallback also does,
        // then a merge may be required; delay until we resolve.

        auto unmerged_values = fallback->unmerged_values();
        stack.insert(stack.end(), make_move_iterator(unmerged_values.begin()), make_move_iterator(unmerged_values.end()));
        auto merged = config_object::merge_origins(stack);
        return construct_delayed_merge(merged, move(stack));
    }

    shared_value config_value::merged_with_the_unmergeable(std::shared_ptr<const unmergeable> fallback) const {
        require_not_ignoring_fallbacks();

        return merged_with_the_unmergeable({ shared_from_this() }, move(fallback));
    }

    shared_value config_value::merged_with_object(vector<shared_value> stack, shared_object fallback) const {
        require_not_ignoring_fallbacks();

        if (dynamic_cast<const config_object*>(this)) {
            throw config_exception(_("Objects must reimplement merged_with_object"));
        }

        return merged_with_non_object(move(stack), move(fallback));
    }

    shared_value config_value::merged_with_non_object(vector<shared_value> stack, shared_value fallback) const {
        require_not_ignoring_fallbacks();

        if (get_resolve_status() == resolve_status::RESOLVED) {
            // falling back to a non-object doesn't merge anything, and also
            // prohibits merging any objects that we fall back to later.
            // so we have to switch to ignoresFallbacks mode.
            return with_fallbacks_ignored();
        } else {
            // if unresolved, we may have to look back to fallbacks as part of
            // the resolution process, so always delay
            return delay_merge(move(stack), move(fallback));
        }
    }

    shared_value config_value::merged_with_non_object(shared_value fallback) const {
        require_not_ignoring_fallbacks();

        return merged_with_non_object({shared_from_this()}, move(fallback));
    }

    shared_value config_value::merged_with_object(shared_object fallback) const {
        require_not_ignoring_fallbacks();

        return merged_with_object({shared_from_this()}, move(fallback));
    }

    shared_value config_value::to_fallback_value() const {
        return shared_from_this();
    }

    shared_value config_value::delay_merge(std::vector<shared_value> stack, shared_value fallback) const {
        // if we turn out to be an object, and the fallback also does,
        // then a merge may be required.
        // if we contain a substitution, resolving it may need to look
        // back to the fallback.
        stack.push_back(move(fallback));
        auto merged = config_object::merge_origins(stack);
        return construct_delayed_merge(merged, move(stack));
    }

}  // namespace hocon
