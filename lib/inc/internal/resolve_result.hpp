#pragma once
#include <internal/resolve_context.hpp>
#include <memory>

namespace hocon {

    template<typename V>
    struct resolve_result {
        resolve_result(resolve_context c, V v) :
            context(std::move(c)), value(std::move(v)) {}

        resolve_context context;
        V value;
    };

    template<typename T>
    static resolve_result<shared_value> make_resolve_result(resolve_context context, T value) {
        return resolve_result<shared_value>(std::move(context), std::move(value));
    }

}  // namespace hocon
