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

    protected:
        virtual shared_value attempt_peek_with_partial_resolve(std::string const& key) const = 0;
        shared_value peek_path(path desired_path) const;
        shared_value peek_assuming_resolved(std::string const& key, path original_path) const;
        shared_value construct_delayed_merge(shared_origin origin, std::vector<shared_value> stack) const override;

        virtual bool is_empty() const = 0;
        virtual std::unordered_map<std::string, shared_value> const& entry_set() const = 0;
        virtual shared_object without_path(path raw_path) const = 0;
        virtual shared_object with_only_path(path raw_path) const = 0;
        virtual shared_object with_only_path_or_null(path raw_path) const = 0;

        static shared_value peek_path(const config_object* self, path desired_path);
        static shared_origin merge_origins(std::vector<shared_value> const& stack);
    };

}  // namespace hocon
