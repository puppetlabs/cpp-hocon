#pragma once
#include <internal/resolve_context.hpp>
#include <memory>

namespace hocon {

    template<typename V>
    struct resolve_result {
        resolve_result(std::shared_ptr<const resolve_context> c, V v) :
            context(std::move(c)), value(std::move(v)) {}

        std::shared_ptr<const resolve_context> context;
        V value;
    };

}  // namespace hocon
