#pragma once

#include <hocon/config_object.hpp>
#include "abstract_config_value.hpp"

namespace hocon {

    class abstract_config_object : public abstract_config_value, public config_object {
        abstract_config_object(shared_origin origin);
    };

}  // namespace hocon

