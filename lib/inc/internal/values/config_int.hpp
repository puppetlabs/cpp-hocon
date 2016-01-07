#pragma once

#include "config_number.hpp"

namespace hocon {

    class config_int : public config_number {
    public:
        config_int(config_origin origin, int value, std::string original_text);

        std::string transform_to_string() const override;

        int64_t long_value() const override;
        double double_value() const override;

    private:
        int _value;
    };

}
