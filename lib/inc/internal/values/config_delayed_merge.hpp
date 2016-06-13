#include <hocon/config_value.hpp>
#include <internal/unmergeable.hpp>
#include <internal/replaceable_merge_stack.hpp>
#include <vector>

#pragma once

namespace hocon {

    class config_delayed_merge : public config_value, public unmergeable, public replaceable_merge_stack {
    public:
        config_delayed_merge(shared_origin origin, std::vector<shared_value> stack);

        config_value::type value_type() const override;

        shared_value make_replacement(resolve_context const& context, int skipping) const override;

        // static method also used by ConfigDelayedMergeObject; end may be null
        static shared_value make_replacement(resolve_context const& context,
                                             std::vector<shared_value> stack,
                                             int skipping);

        std::vector<shared_value> unmerged_values() const override;

        unwrapped_value unwrapped() const override;

        resolve_result<shared_value> resolve_substitutions(resolve_context const& context, resolve_source const& source) const override;
        static resolve_result<shared_value> resolve_substitutions(std::shared_ptr<const replaceable_merge_stack> replaceable, const std::vector<shared_value>& _stack, resolve_context const& context, resolve_source const& source);
        resolve_status get_resolve_status() const override { return resolve_status::UNRESOLVED; }

        bool operator==(config_value const& other) const override;

        shared_value replace_child(shared_value const& child, shared_value replacement) const override;
        bool has_descendant(shared_value const& descendant) const override;

    protected:
        shared_value new_copy(shared_origin) const override;

        bool ignores_fallbacks() const override;

    private:
        std::vector<shared_value> _stack;
    };

}  // namespace hocon::config_delayed_merge

