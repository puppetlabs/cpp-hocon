#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_origin.hpp>

#include <string>

namespace hocon {

    /**
     * This exists because sometimes null is not the same as missing. Specifically,
     * if a value is set to null we can give a better error message (indicating
     * where it was set to null) in case someone asks for the value. Also, null
     * overrides values set "earlier" in the search path, while missing values do
     * not.
     */
    class config_null : public config_value {
    public:
        config_null(config_origin origin);

        config_value_type value_type() const override;
        std::string transform_to_string() const override;
    };

}  // namespace hocon
