#pragma once

#include <hocon/config_value.hpp>
#include <internal/simple_config_origin.hpp>

namespace hocon {

    class abstract_config_value : public config_value {
    public:
        abstract_config_value(simple_config_origin origin);

        virtual std::string transform_to_string() const;

    private:
        simple_config_origin _origin;
    };

}  // namespace hocon
