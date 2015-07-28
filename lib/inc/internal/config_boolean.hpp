#pragma once

#include "abstract_config_value.hpp"

namespace hocon {

    class config_boolean : public abstract_config_value {
    public:
        config_boolean(simple_config_origin origin, bool value);

        config_value_type value_type() const override;
        std::string transform_to_string() const override;

    private:
        bool _value;
    };
}  // namespace hocon;

