#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_mergeable.hpp>

namespace hocon {

    class mergeable_value : public config_mergeable {
    public:
        // converts a Config to its root object and a ConfigValue to itself
        virtual shared_value to_fallback_value() const = 0;
    };

}  // namespace hocon::mergeable_value

