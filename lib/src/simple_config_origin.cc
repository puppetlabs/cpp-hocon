#include <internal/simple_config_origin.hpp>
#include <stdexcept>

using namespace std;

namespace hocon {

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type, string resource, vector<string> comments) :
        _description(move(description)), _line_number(line_number), _end_line_number(end_line_number),
        _origin_type(org_type), _resource_or_null(move(resource)), _comments_or_null(move(comments))
    {
        if (_description.empty()) {
            throw runtime_error("description may not be null");
        }
    }

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type) :
        _description(move(description)), _line_number(line_number), _end_line_number(end_line_number),
        _origin_type(org_type)
    {
        if (_description.empty()) {
            throw runtime_error("description may not be null");
        }
    }

    int simple_config_origin::line_number() const {
        return _line_number;
    }

    shared_origin simple_config_origin::with_line_number(int line_number) const {
        return make_shared<simple_config_origin>(_description, line_number, line_number, _origin_type,
                    _resource_or_null, _comments_or_null);
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

}  // namespace hocon
