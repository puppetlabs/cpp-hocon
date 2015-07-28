#include <internal/simple_config_origin.hpp>

using namespace std;

namespace hocon {

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type, string resource_or_null,
                                               vector<string> comments_or_null) {
        if (description.empty()) {
            throw runtime_error("description may not be null");
        }

        _description = description;
        _line_number = line_number;
        _end_line_number = end_line_number;
        _origin_type = org_type;
        _resource_or_null = resource_or_null;
        _comments_or_null = comments_or_null;
    }

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type) {
        if (description.empty()) {
            throw runtime_error("description may not be null");
        }

        _description = description;
        _line_number = line_number;
        _end_line_number = end_line_number;
        _origin_type = org_type;
    }

    bool simple_config_origin::operator==(const simple_config_origin &other) const {
        return (other._description == _description) &&
                (other._line_number == _line_number) &&
                (other._end_line_number == _end_line_number) &&
                (other._origin_type == _origin_type) &&
                (other._resource_or_null == _resource_or_null) &&
                (other._comments_or_null == _comments_or_null);
    }

    bool simple_config_origin::operator!=(const simple_config_origin &other) const {
        return !(*this == other);
    }

    simple_config_origin* simple_config_origin::new_simple(string description) {
        return new simple_config_origin(description, -1, -1, origin_type::GENERIC);
    }

}  // namespace hocon
