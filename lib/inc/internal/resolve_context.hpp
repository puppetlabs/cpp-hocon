#pragma once

#include <hocon/types.hpp>
#include <hocon/config_resolve_options.hpp>

namespace hocon {

    class resolve_source;

    template<typename T>
    struct resolve_result;

    // TODO: Implement this class >_>
    class resolve_context {
    public:
        bool is_restricted_to_child() const;
        config_resolve_options options() const;

        resolve_result<shared_value> resolve(shared_value original, resolve_source const& source) const;

    private:
        config_resolve_options _options;
    };
}  // namespace hocon
