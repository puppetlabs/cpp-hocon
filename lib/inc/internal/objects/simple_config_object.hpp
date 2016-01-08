#pragma once

#include <hocon/config_object.hpp>
#include <hocon/config.hpp>

namespace hocon {

    class simple_config_object : public config_object {
    public:
        simple_config_object(config_origin origin, std::unordered_map<std::string, shared_value> value,
                             resolve_status status = resolve_status::RESOLVED, bool ignores_fallbacks = false);

        shared_value attempt_peek_with_partial_resolve(std::string const& key) const override;

        bool is_empty() const override;
        std::unordered_map<std::string, shared_value> const& entry_set() const override;

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

    private:
        std::unordered_map<std::string, shared_value> _value;
        // TODO: Put these back when needed, currently cause an unused error
//        bool _resolved;
        bool _ignores_fallbacks;
    };

}  // namespace hocon
