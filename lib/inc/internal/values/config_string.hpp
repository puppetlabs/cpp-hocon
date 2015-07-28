#pragma once

#include "abstract_config_value.hpp"

namespace hocon {

    enum class config_string_type { QUOTED, UNQUOTED };

    class config_string : public abstract_config_value {
    public:
        config_string(std::shared_ptr<simple_config_origin> origin, std::string text, config_string_type quoted);

        config_value_type value_type() const override;
        std::string transform_to_string() const override;

        bool was_quoted() const;

    private:
        std::string _text;
        config_string_type _quoted;
    };

}  // namespace hocon
