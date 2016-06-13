#pragma once

#include <hocon/config_value.hpp>

namespace hocon {

    enum class config_string_type { QUOTED, UNQUOTED };

    class config_string : public config_value {
    public:
        config_string(shared_origin origin, std::string text, config_string_type quoted);

        config_value::type value_type() const override;
        std::string transform_to_string() const override;

        unwrapped_value unwrapped() const override;

        bool was_quoted() const;
        bool operator==(config_value const& other) const override;

    protected:
        shared_value new_copy(shared_origin) const override;

    private:
        std::string _text;
        config_string_type _quoted;
    };

}  // namespace hocon
