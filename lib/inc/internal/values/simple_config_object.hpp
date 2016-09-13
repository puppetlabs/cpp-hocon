#pragma once

#include <internal/container.hpp>
#include <hocon/config_object.hpp>
#include <hocon/config_value.hpp>
#include <hocon/config.hpp>
#include <unordered_map>

namespace hocon {

    class simple_config_object : public config_object, public container {
    public:
        simple_config_object(shared_origin origin, std::unordered_map<std::string, shared_value> value,
                             resolve_status status, bool ignores_fallbacks);

        simple_config_object(shared_origin origin, std::unordered_map<std::string, shared_value> value);

        shared_value attempt_peek_with_partial_resolve(std::string const& key) const override;

        // map interface
        bool is_empty() const override { return _value.empty(); }
        size_t size() const override { return _value.size(); }
        shared_value operator[](std::string const& key) const override { return _value.at(key); }
        iterator begin() const override { return _value.begin(); }
        iterator end() const override { return _value.end(); }
        unwrapped_value unwrapped() const override;

        shared_value get(std::string const& key) const override {
            if (_value.find(key) == _value.end()) {
                return nullptr;
            }
            return _value.at(key);
        }

        std::unordered_map<std::string, shared_value> const& entry_set() const override;

        resolve_status get_resolve_status() const override { return _resolved; }
        bool ignores_fallbacks() const override { return _ignores_fallbacks; }
        shared_value with_fallbacks_ignored() const override;
        shared_value merged_with_object(shared_object fallback) const override;

        shared_object with_value(path raw_path, shared_value value) const override;
        shared_object with_value(std::string key, shared_value value) const override;
        shared_object without_path(path raw_path) const override;
        shared_object with_only_path(path raw_path) const override;

        /**
         * Gets the object with only the path if the path
         * exists, otherwise null if it doesn't. this ensures
         * that if we have { a : { b : 42 } } and do
         * withOnlyPath("a.b.c") that we don't keep an empty
         * "a" object.
         */
        shared_object with_only_path_or_null(path raw_path) const override;

        /**
         * Replace a child of this value. CAUTION if replacement is null, delete the
         * child, which may also delete the parent, or make the parent into a
         * non-container.
         */
        shared_value replace_child(shared_value const& child, shared_value replacement) const override;

        /**
         * Super-expensive full traversal to see if descendant is anywhere
         * underneath this container.
         */
        bool has_descendant(shared_value const& descendant) const override;

        /**
         * Construct a list of keys in the _value map.
         * Use a vector rather than set, because most of the time we just want to iterate over them.
         */
        std::vector<std::string> key_set() const override;

        /**
         * Construct a list of the values from the provided map.
         * Equivalent to Java's Collection.values() method.
         */
        std::vector<shared_value> value_set(std::unordered_map<std::string, shared_value> m) const;

        bool operator==(config_value const& other) const override;

        static std::shared_ptr<simple_config_object> empty();
        static std::shared_ptr<simple_config_object> empty(shared_origin origin);
        static std::shared_ptr<simple_config_object> empty_instance();

    protected:
        resolve_result<shared_value>
            resolve_substitutions(resolve_context const& context, resolve_source const& source) const override;
        shared_value new_copy(shared_origin) const override;
        void render(std::string& s, int indent, bool at_root, config_render_options options) const override;

    private:
        std::unordered_map<std::string, shared_value> _value;
        resolve_status _resolved;
        bool _ignores_fallbacks;

        shared_object new_copy(resolve_status const& new_status, shared_origin new_origin) const override;
        std::shared_ptr<simple_config_object> modify(no_exceptions_modifier& modifier) const;
        std::shared_ptr<simple_config_object> modify_may_throw(modifier& modifier) const;

        static resolve_status resolve_status_from_value(const std::unordered_map<std::string, shared_value>& value);

        struct resolve_modifier;
    };

}  // namespace hocon
