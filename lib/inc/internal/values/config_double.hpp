#pragma once

#include "config_number.hpp"
#include <hocon/config_origin.hpp>

#include <string>

namespace hocon {

    class config_double : public config_number {
    public:
        config_double(config_origin origin, double value, std::string original_text);

        std::string transform_to_string() const override;

        int64_t long_value() const override;
        double double_value() const override;

    private:
        double _value;
    };

}  // namespace hocon
