#pragma once

#include <hocon/config_value.hpp>

namespace hocon {

    class default_transformer {
    public:
        static shared_value transform(shared_value value, config_value_type requested);
    };

}  // namespace hocon
