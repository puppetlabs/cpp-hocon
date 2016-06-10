#pragma once

#include "config_value.hpp"
#include "config_mergeable.hpp"
#include "path.hpp"
#include <unordered_map>
#include "export.h"

namespace hocon {

    class LIBCPP_HOCON_EXPORT config_object : public config_value {
        friend class config;
        friend class config_value;
        friend class simple_config_object;
        friend class resolve_source;
        friend class config_delayed_merge_object;
    public:
        /**
         * Converts this object to a {@link Config} instance, enabling you to use
         * path expressions to find values in the object. This is a constant-time
         * operation (it is not proportional to the size of the object).
         *
         * @return a {@link Config} with this object as its root
         */
        virtual std::shared_ptr<const config> to_config() const;

        config_object(shared_origin origin);

        config_value::type value_type() const override;

        virtual shared_object with_value(path raw_path, shared_value value) const = 0;
        virtual shared_object with_value(std::string key, shared_value value) const = 0;

        /**
         * Look up the key on an only-partially-resolved object, with no
         * transformation or type conversion of any kind; if 'this' is not resolved
         * then try to look up the key anyway if possible.
         *
         * @param key
         *            key to look up
         * @return the value of the key, or null if known not to exist
         * @throws config_exception
         *             if can't figure out key's value (or existence) without more
         *             resolving
         */
        virtual shared_value attempt_peek_with_partial_resolve(std::string const& key) const = 0;

        // map interface
        using iterator = std::unordered_map<std::string, shared_value>::const_iterator;
        virtual bool is_empty() const = 0;
        virtual size_t size() const = 0;
        virtual shared_value operator[](std::string const& key) const = 0;
        virtual shared_value get(std::string const& key) const = 0;
        virtual iterator begin() const = 0;
        virtual iterator end() const = 0;

    protected:
        shared_value peek_path(path desired_path) const;
        shared_value peek_assuming_resolved(std::string const& key, path original_path) const;

        virtual shared_object new_copy(resolve_status const& status, shared_origin origin) const = 0;
        shared_value new_copy(shared_origin origin) const override;

        shared_value construct_delayed_merge(shared_origin origin, std::vector<shared_value> stack) const override;

        virtual std::unordered_map<std::string, shared_value> const& entry_set() const = 0;
        virtual shared_object without_path(path raw_path) const = 0;
        virtual shared_object with_only_path(path raw_path) const = 0;
        virtual shared_object with_only_path_or_null(path raw_path) const = 0;

        static shared_value peek_path(const config_object* self, path desired_path);
        static shared_origin merge_origins(std::vector<shared_value> const& stack);
    };

}  // namespace hocon
