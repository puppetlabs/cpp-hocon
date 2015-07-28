#pragma once

#include <hocon/config_value.hpp>
#include <internal/simple_config_origin.hpp>

namespace hocon {

    class abstract_config_value : public config_value {
    public:
        abstract_config_value(simple_config_origin origin);

        virtual std::string transform_to_string();
        virtual config_value_type value_type() = 0;
        virtual abstract_config_value* new_copy(simple_config_origin origin) = 0;

    private:
        simple_config_origin _origin;
    };

}  // namespace hocon
