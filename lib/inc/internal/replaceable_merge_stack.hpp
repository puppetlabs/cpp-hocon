#pragma once

#include <internal/resolve_context.hpp>
#include <internal/container.hpp>

namespace hocon {

    // This is a faithful port of the Java ReplaceableMergeStack interface
    class replaceable_merge_stack : public container {
    public:
        virtual shared_value make_replacement(resolve_context const& context, int skipping) const = 0;
    };

}  // namespace hocon
