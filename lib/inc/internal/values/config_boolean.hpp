#pragma once

#include <hocon/config_value.hpp>

namespace hocon {

    class config_boolean : public config_value {
    public:
        config_boolean(shared_origin origin, bool value);

        config_value_type value_type() const override;
        std::string transform_to_string() const override;

        bool bool_value() const;

    protected:
        shared_value new_copy(shared_origin) const override;
        bool operator==(config_value const& other) const override;

    private:
        bool _value;
    };
}  // namespace hocon;

