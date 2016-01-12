#pragma once

#include <memory>

namespace hocon {

    class container;

    class resolve_source {
    public:
        resolve_source push_parent(std::shared_ptr<const container> parent) const;
    };
}  // namespace hocon
