#include <hocon/config_origin.hpp>
#include <stdexcept>

using namespace std;

namespace hocon {

    struct config_origin::simple_config_origin {
        simple_config_origin(string desc, int ln, int eln, origin_type ot, string res, vector<string> com) :
            description(move(desc)), line_number(ln), end_line_number(eln),
            type(ot), resource_or_null(move(res)), comments_or_null(move(com))
        {
            if (description.empty()) {
                throw runtime_error("description may not be null");
            }
        }

        bool operator==(const simple_config_origin &other) const
        {
            return (other.description == description) &&
                (other.line_number == line_number) &&
                (other.end_line_number == end_line_number) &&
                (other.type == type) &&
                (other.resource_or_null == resource_or_null) &&
                (other.comments_or_null == comments_or_null);
        }

        std::string description;
        int line_number;
        int end_line_number;
        origin_type type;
        std::string resource_or_null;
        std::vector<std::string> comments_or_null;
    };

    config_origin::config_origin(string description, int line_number, int end_line_number,
                                 origin_type org_type, string resource, vector<string> comments) :
        _impl{make_shared<simple_config_origin>(move(description), line_number, end_line_number, org_type, move(resource), move(comments))}
    {}

    int config_origin::line_number() const {
        return _impl ? _impl->line_number : -1;
    }

    string config_origin::description() const {
        return _impl ? _impl->description : string();
    }

    config_origin config_origin::with_line_number(int line_number) const {
        if (!_impl || (line_number == _impl->line_number && line_number == _impl->end_line_number)) {
            return *this;
        } else {
            return config_origin(_impl->description, line_number, line_number, _impl->type,
                                 _impl->resource_or_null, _impl->comments_or_null);
        }
    }

    config_origin config_origin::append_comments(vector<string> comments) const {
        if (!_impl || comments == _impl->comments_or_null || comments.empty()) {
            return *this;
        } else {
            // Don't re-use with_comments, because we've already checked whether they're equal.
            // If they're not equal now, the concatenated comments won't be equal either.
            comments.insert(comments.begin(), _impl->comments_or_null.begin(), _impl->comments_or_null.end());
            return config_origin(_impl->description, _impl->line_number, _impl->end_line_number,
                                 _impl->type, _impl->resource_or_null, move(comments));
        }
    }

    bool config_origin::operator==(const config_origin &other) const {
        return *other._impl == *_impl;
    }

    bool config_origin::operator!=(const config_origin &other) const {
        return !(*this == other);
    }

}  // namespace hocon

