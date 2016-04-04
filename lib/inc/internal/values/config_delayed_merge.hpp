#include <hocon/config_value.hpp>
#include <internal/unmergeable.hpp>
#include <vector>

#pragma once

namespace hocon {

    class config_delayed_merge : public config_value, public unmergeable {
    public:
        config_delayed_merge(shared_origin origin, std::vector<shared_value> stack);

        config_value::type value_type() const override;
        std::vector<shared_value> unmerged_values() const override;

        resolve_status get_resolve_status() const override { return resolve_status::UNRESOLVED; }

    protected:
        shared_value new_copy(shared_origin) const override;
        bool operator==(config_value const& other) const override;

        bool ignores_fallbacks() const override;

    private:
        std::vector<shared_value> _stack;
    };

}  // namespace hocon::config_delayed_merge

