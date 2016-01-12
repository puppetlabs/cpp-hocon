#pragma once
#include <internal/resolve_context.hpp>

namespace hocon {

    template<typename V>
    struct resolve_result {
        resolve_result(resolve_context c, V v) :
            context(std::move(c)), value(std::move(v)) {}

        resolve_context context;
        V value;
    };

}  // namespace hocon
