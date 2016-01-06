#pragma once
#include <internal/resolve_context.hpp>
#include <memory>

namespace hocon {
    // TODO: yolo
    template<typename V>
    class resolve_result {
    public:
        resolve_result(resolve_context context, V value) :
            _context(std::move(context)), _value(std::move(value)) {}

    private:
        resolve_context _context;
        V _value;
    };
}  // namespace hocon
