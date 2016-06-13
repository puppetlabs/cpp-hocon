#pragma once

#include "config_number.hpp"
#include <internal/simple_config_origin.hpp>

#include <string>

namespace hocon {

    class config_double : public config_number {
    public:
        config_double(shared_origin origin, double value, std::string original_text);

        std::string transform_to_string() const override;

        unwrapped_value unwrapped() const override;

        int64_t long_value() const override;
        double double_value() const override;

    protected:
        shared_value new_copy(shared_origin) const override;

    private:
        double _value;
    };

}  // namespace hocon
