#pragma once

#include <internal/abstract_config_value.hpp>

namespace hocon {

    class config_boolean : public abstract_config_value {
    public:
        config_boolean(simple_config_origin origin, bool value);
        config_value_type value_type();
        config_boolean* new_copy(simple_config_origin origin) override;
        std::string transform_to_string() override;

    private:
        bool _value;
    };
}  // namespace hocon;

