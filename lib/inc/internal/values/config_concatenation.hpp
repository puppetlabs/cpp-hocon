#pragma once

#include <hocon/config_value.hpp>
#include <internal/unmergeable.hpp>
#include <internal/container.hpp>
#include <string>
#include <vector>

namespace hocon {

    struct config_exception;

    /**
     * A ConfigConcatenation represents a list of values to be concatenated (see the
     * spec). It only has to exist if at least one value is an unresolved
     * substitution, otherwise we could go ahead and collapse the list into a single
     * value.
     *
     * Right now this is always a list of strings and ${} references, but in the
     * future should support a list of ConfigList. We may also support
     * concatenations of objects, but ConfigDelayedMerge should be used for that
     * since a concat of objects really will merge, not concatenate.
     */
    class config_concatenation : public config_value, public unmergeable, public container {
    public:
        config_concatenation(shared_origin origin, std::vector<shared_value> pieces);

        config_value::type value_type() const override;
        std::vector<shared_value> unmerged_values() const override;

        resolve_status get_resolve_status() const override;

        shared_value replace_child(shared_value const& child, shared_value replacement) const override;
        bool has_descendant(shared_value const& descendant) const override;
        resolve_result<shared_value> resolve_substitutions(resolve_context const& context, resolve_source const& source) const override;

        static std::vector<shared_value> consolidate(std::vector<shared_value> pieces);
        static shared_value concatenate(std::vector<shared_value> pieces);
        shared_value relativized(std::string prefix) const override;

        unwrapped_value unwrapped() const override;

        bool operator==(config_value const& other) const override;

    protected:
        shared_value new_copy(shared_origin origin) const override;
        bool ignores_fallbacks() const override;
        void render(std::string& result, int indent, bool at_root, config_render_options options) const override;

    private:
        std::vector<shared_value> _pieces;

        config_exception not_resolved() const;
        static bool is_ignored_whitespace(shared_value value);
        static void join(std::vector<shared_value> & builder, shared_value right);
    };

}  // namespace hocon

