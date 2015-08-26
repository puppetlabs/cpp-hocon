#pragma once

#include <hocon/config_origin.hpp>

#include <string>
#include <vector>
#include <memory>

namespace hocon {

    enum class origin_type { GENERIC, FILE, RESOURCE };

    class simple_config_origin : public config_origin {
    public:
        simple_config_origin(std::string description, int line_number, int end_line_number,
            origin_type org_type, std::string resource_or_null, std::vector<std::string> comments_or_null);

        /** This constructor replaces the new_simple method in the original library. */
        simple_config_origin(std::string description, int line_number = -1, int end_line_number = -1,
                             origin_type org_type = origin_type::GENERIC);

        int line_number() const override;

        std::string description() const;

        /**
         * Returns a pointer to a copy of this origin with the specified line number
         * as both starting and ending line.
         */
        std::shared_ptr<const simple_config_origin> with_line_number(int line_number) const;

        bool operator==(const simple_config_origin &other) const;
        bool operator!=(const simple_config_origin &other) const;

    private:
        std::string _description;
        int _line_number;
        int _end_line_number;
        origin_type _origin_type;
        std::string _resource_or_null;
        std::vector<std::string> _comments_or_null;
    };

    using shared_origin = std::shared_ptr<const simple_config_origin>;

}  // namespace hocon
