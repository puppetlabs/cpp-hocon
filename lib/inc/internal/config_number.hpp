#pragma once

#include <internal/abstract_config_value.hpp>
#include <internal/simple_config_origin.hpp>

#include <string>

namespace hocon {

    class config_number : public abstract_config_value {
    public:
        config_number(simple_config_origin origin, std::string original_text);

        std::string transform_to_string();
        config_value_type value_type();

        virtual long long_value() const = 0;
        virtual double double_value() const = 0;
        bool is_whole() const;

        bool operator==(const config_number &other) const;
        bool operator!=(const config_number &other) const;

    private:
        std::string _original_text;
    };

}  // namespace hocon
