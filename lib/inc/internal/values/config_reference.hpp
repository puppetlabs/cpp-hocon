#pragma once

#include <hocon/config_value.hpp>
#include <internal/unmergeable.hpp>

namespace hocon {

    class substitution_expression;

    class config_reference : public config_value, public unmergeable {
    public:
        config_reference(shared_origin origin, std::shared_ptr<substitution_expression> expr, int prefix_length = 0);

        /**
        * The type of the value; matches the JSON type schema.
        *
        * @return value's type
        */
        type value_type() const override;
        std::vector<shared_value> unmerged_values() const override;
        resolve_status get_resolve_status() const override;

    protected:
        shared_value new_copy(shared_origin origin) const override;
        bool operator==(config_value const& other) const override;
        resolve_result<shared_value> resolve_substitutions(resolve_context const& context, resolve_source const& source) const override;
        bool ignores_fallbacks() const override { return false; }

    private:
        std::shared_ptr<substitution_expression> _expr;
        int _prefix_length;
    };
}
