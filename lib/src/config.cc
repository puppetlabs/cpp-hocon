#include <hocon/config.hpp>
#include <internal/values/config_null.hpp>
#include <internal/config_exception.hpp>
#include <internal/default_transformer.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_number.hpp>
#include <internal/values/config_string.hpp>

using namespace std;

namespace hocon {

    config::config(shared_object object) : _object(move(object)) { }

    shared_object config::root() const {
        return _object;
    }

    shared_origin config::origin() const {
        return _object->origin();
    }

    shared_config config::resolve() const {
        return resolve(config_resolve_options());
    }

    shared_config config::resolve(config_resolve_options options) const {
        return resolve_with(shared_from_this(), options);
    }

    shared_config config::resolve_with(shared_config source) const {
        return resolve_with(source, config_resolve_options());
    }

    shared_config config::resolve_with(shared_config source, config_resolve_options options) const {
        // TODO: implement all the resolve classes, then this method
        // until then DO NOT TRY TO PARSE INPUT WITH SUBSTITUTIONS
        return shared_from_this();
    }

    shared_value config::has_path_peek(string const& path_expression) const {
        path raw_path = path::new_path(path_expression);
        shared_value peeked;
        try {
            peeked = _object->peek_path(raw_path);
        } catch (config_exception& ex) {
            throw config_exception(raw_path.render() + " has not been resolved, you need to call config::resolve()");
        }
        return peeked;
    }

    bool config::has_path(string const& path_expression) const {
        shared_value peeked = has_path_peek(path_expression);
        return peeked && peeked->value_type() != config_value_type::CONFIG_NULL;
    }

    bool config::has_path_or_null(string const& path) const {
        shared_value peeked = has_path_peek(path);
        return peeked != nullptr;
    }

    bool config::is_empty() const {
        return _object->is_empty();
    }

    void config::find_paths(set<pair<string, shared_ptr<const config_value>>>& entries, path parent,
                                   shared_object obj) {
        for (auto&& entry : obj->entry_set()) {
            string elem = entry.first;
            shared_value v = entry.second;
            path new_path = path::new_key(elem);
            if (!parent.empty()) {
                new_path = new_path.prepend(parent);
            }
            if (auto object = dynamic_pointer_cast<const config_object>(v)) {
                find_paths(entries, new_path, object);
            } else if (auto null_value = dynamic_pointer_cast<const config_null>(v)) {
                // nothing; nulls are conceptually not in a config
            } else {
                entries.insert(make_pair(new_path.render(), v));
            }
        }
    }

    set<pair<string, shared_ptr<const config_value>>> config::entry_set() const {
        set<pair<string, shared_ptr<const config_value>>> entries;
        find_paths(entries, path(), _object);
        return entries;
    }

    shared_value config::throw_if_null(shared_value v, config_value_type expected, path original_path) {
        if (v->value_type() == config_value_type::CONFIG_NULL) {
            throw config_exception(original_path.render() + " was null");
        } else {
            return v;
        }
    }

    shared_value config::find_key(shared_object self, string const& key, config_value_type expected,
                                         path original_path) {
        return throw_if_null(find_key_or_null(self, key, expected, original_path), expected, original_path);
    }

    shared_value config::find_key_or_null(shared_object self, string const& key, config_value_type expected,
                                                 path original_path) {
        shared_value v = self->peek_assuming_resolved(key, original_path);
        if (!v) {
            throw config_exception("Value missing at " + original_path.render());
        }

        if (expected != config_value_type::UNSPECIFIED) {
            v = default_transformer::transform(v, expected);
        }

        if (expected != config_value_type::UNSPECIFIED &&
                v->value_type() != expected &&
                v->value_type() != config_value_type::CONFIG_NULL) {
            throw config_exception(original_path.render() + " could not be converted to the requested type");
        } else {
            return v;
        }
    }

    shared_value config::find_or_null(shared_object self, path desired_path,
                                             config_value_type expected, path original_path) {
        try {
            string key = *desired_path.first();
            path next = desired_path.remainder();
            if (next.empty()) {
                return find_key_or_null(self, key, expected, original_path);
            } else {
                shared_object o = dynamic_pointer_cast<const config_object>(
                        find_key(self, key, config_value_type::OBJECT,
                                 original_path.sub_path(0, original_path.length() - next.length())));
                return find_or_null(o, next, expected, original_path);
            }
        } catch (config_exception& ex) {
            throw config_exception(desired_path.render() +
                                           " has not been resolved, you need to call config::resolve()");
        }
    }

    shared_value config::find_or_null(string const& path_expression, config_value_type expected) const {
        path raw_path = path::new_path(path_expression);
        return find_or_null(raw_path, expected, raw_path);
    }

    shared_value config::find(string const& path_expression, config_value_type expected) const {
        path raw_path = path::new_path(path_expression);
        return find(raw_path, expected, raw_path);
    }

    bool config::get_is_null(string const& path_expression) const {
        shared_value v = find_or_null(path_expression, config_value_type::UNSPECIFIED);
        return v->value_type() == config_value_type::CONFIG_NULL;
    }

    shared_ptr<const config_value> config::get_value(string const& path_expression) const {
        return find(path_expression, config_value_type::UNSPECIFIED);
    }

    bool config::get_bool(string const& path_expression) const {
        shared_value v = find(path_expression, config_value_type::BOOLEAN);
        return dynamic_pointer_cast<const config_boolean>(v)->bool_value();
    }

    int config::get_int(string const& path_expression) const {
        shared_value v = find(path_expression, config_value_type::NUMBER);
        return dynamic_pointer_cast<const config_number>(v)->int_value_range_checked(path_expression);
    }

    int64_t config::get_long(string const& path_expression) const {
        shared_value v = find(path_expression, config_value_type::NUMBER);
        return dynamic_pointer_cast<const config_number>(v)->long_value();
    }

    double config::get_double(string const& path_expression) const {
        shared_value v = find(path_expression, config_value_type::NUMBER);
        return dynamic_pointer_cast<const config_number>(v)->double_value();
    }

    string config::get_string(string const& path_expression) const {
        shared_value v = find(path_expression, config_value_type::STRING);
        return dynamic_pointer_cast<const config_string>(v)->transform_to_string();
    }

    shared_ptr<const config_object> config::get_object(string const& path_expression) const {
        return dynamic_pointer_cast<const config_object>(find(path_expression, config_value_type::OBJECT));
    }

    shared_config config::get_config(string const& path_expression) const {
        return get_object(path_expression)->to_config();
    }

    shared_value config::to_fallback_value() const {
        return _object;
    }

    shared_ptr<const config_mergeable> config::with_fallback(shared_ptr<const config_mergeable> other) const {
        // TODO: this isn't technically the same functionality as the Java, but full
        // functionality requires having merging working, see AbstractConfigValue#withFallback
        return dynamic_pointer_cast<const config_mergeable>(shared_from_this());
    }

    bool config::operator==(config const &other) const {
        return _object == other._object;
    }

    template<typename T>
    vector<T> config::get_homogeneous_unwrapped_list(string const& path, config_value_type expected) const {
        // TODO: this relies on complex config_value types
        // Also will need to some design decisions about getting naked values of type T
        // out of anonymous config values... possible a templated version of the Java's unwrapped method?
        throw runtime_error("not implemented");
    }

    shared_config config::with_value(string const& path_expression, shared_ptr<const config_value> value) const {
        path raw_path = path::new_path(path_expression);
        return make_shared<config>(root()->with_value(raw_path, value));
    }

    bool config::is_resolved() const {
        return root()->get_resolve_status() == resolve_status::RESOLVED;
    }

    shared_config config::without_path(string const& path_expression) const {
        path raw_path = path::new_path(path_expression);
        return make_shared<config>(root()->without_path(raw_path));
    }

    shared_config config::with_only_path(string const& path_expression) const {
        path raw_path = path::new_path(path_expression);
        return make_shared<config>(root()->with_only_path(raw_path));
    }

    shared_config config::at_key(shared_origin origin, string const& key) const {
        return root()->at_key(origin, key);
    }

    shared_config config::at_key(std::string const& key) const {
        return root()->at_key(key);
    }

    shared_config config::at_path(std::string const& path) const {
        return root()->at_path(path);
    }

    void config::check_valid(shared_config reference, std::vector<std::string> restrict_to_paths) const {
        // TODO: implement this once resolve functionality is working
        throw runtime_error("Method not implemented");
    }

    shared_value config::peek_path(path desired_path) const {
        return root()->peek_path(desired_path);
    }

    shared_value config::find_or_null(path path_expression, config_value_type expected,
            path original_path) const {
        return find_or_null(_object, path_expression, expected, original_path);
    }

    shared_value config::find(path path_expression, config_value_type expected, path original_path) const {
        return throw_if_null(find_or_null(_object, path_expression, expected, original_path), expected, original_path);
    }

}  // namespace hocon
