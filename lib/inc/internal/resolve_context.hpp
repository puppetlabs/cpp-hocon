#pragma once

#include <hocon/types.hpp>
#include <hocon/config_resolve_options.hpp>
#include <hocon/path.hpp>

namespace hocon {

    class resolve_source;

    template<typename T>
    struct resolve_result;

    // TODO: Implement this class >_>
    class resolve_context {
    public:
        resolve_context(config_resolve_options options, path restrict_to_child);
        bool is_restricted_to_child() const;
        config_resolve_options options() const;

        resolve_result<shared_value> resolve(shared_value original, resolve_source const& source) const;
        path restrict_to_child() const;

        resolve_context restrict(path restrict_to) const;
        resolve_context unrestricted() const;

        static shared_value resolve(shared_value value, shared_object root, config_resolve_options options);

    private:
        config_resolve_options _options;
        path _restrict_to_child;
    };
}  // namespace hocon
