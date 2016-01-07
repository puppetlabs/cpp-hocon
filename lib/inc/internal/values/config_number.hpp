#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_origin.hpp>

#include <string>

namespace hocon {

    class config_number : public config_value {
    public:
        config_number(config_origin origin,
                      std::string original_text);

        std::string transform_to_string() const;
        config_value_type value_type() const;

        virtual int64_t long_value() const = 0;
        virtual double double_value() const = 0;
        bool is_whole() const;

        bool operator==(const config_number &other) const;
        bool operator!=(const config_number &other) const;

        int int_value_range_checked(std::string const& path) const;

        static std::shared_ptr<config_number> new_number(
                config_origin origin, int64_t value, std::string original_text);

        static std::shared_ptr<config_number> new_number(
                config_origin origin, double value, std::string original_text);

    private:
        std::string _original_text;
    };

}  // namespace hocon
