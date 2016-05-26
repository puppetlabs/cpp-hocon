#pragma once

#include <internal/resolve_context.hpp>

namespace hocon {

    // This is a faithful port of the Java ReplaceableMergeStack interface
    class replaceable_merge_stack {
    public:
        virtual shared_value make_replacement(resolve_context const& context, int skipping) const = 0;
    };

}  // namespace hocon
