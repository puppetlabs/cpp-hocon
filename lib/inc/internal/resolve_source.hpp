#pragma once

#include <memory>
#include <hocon/types.hpp>

namespace hocon {

    class container;

    class resolve_source {
    public:
        resolve_source(shared_object root);
        resolve_source push_parent(std::shared_ptr<const container> parent) const;

    private:
        std::shared_ptr<const container> _path_from_root;
        shared_object _root;
    };
}  // namespace hocon
