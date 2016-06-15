#pragma once

#include "config_number.hpp"

namespace hocon {

    class config_int : public config_number {
    public:
        config_int(shared_origin origin, int value, std::string original_text);

        std::string transform_to_string() const override;

        unwrapped_value unwrapped() const override;

        int64_t long_value() const override;
        double double_value() const override;

    protected:
        shared_value new_copy(shared_origin) const override;

    private:
        int _value;
    };

}
