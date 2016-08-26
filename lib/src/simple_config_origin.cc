#include <hocon/config_value.hpp>
#include <internal/simple_config_origin.hpp>
#include <hocon/config_exception.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <leatherman/locale/locale.hpp>
#include <algorithm>
#include <stdexcept>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type, string resource, vector<string> comments) :
        _description(move(description)), _line_number(line_number), _end_line_number(end_line_number),
        _origin_type(org_type), _resource_or_null(move(resource)), _comments_or_null(move(comments)) { }

    simple_config_origin::simple_config_origin(string description, int line_number, int end_line_number,
                                               origin_type org_type) :
        _description(move(description)), _line_number(line_number), _end_line_number(end_line_number),
        _origin_type(org_type) { }

    int simple_config_origin::line_number() const {
        return _line_number;
    }

    string const& simple_config_origin::description() const {
        return _description;
    }

    vector<string> const& simple_config_origin::comments() const {
        return _comments_or_null;
    }

    shared_origin simple_config_origin::with_line_number(int line_number) const {
        if (line_number == _line_number && line_number == _end_line_number) {
            return shared_from_this();
        } else {
            return make_shared<simple_config_origin>(_description, line_number, line_number, _origin_type,
                    _resource_or_null, _comments_or_null);
        }
    }

    shared_origin simple_config_origin::with_comments(std::vector<std::string> comments) const {
        if (comments == _comments_or_null || comments.empty()) {
            return shared_from_this();
        } else {
            return make_shared<simple_config_origin>(_description, _line_number, _end_line_number, _origin_type,
                                                     _resource_or_null, move(comments));
        }
    }

    shared_ptr<const simple_config_origin> simple_config_origin::append_comments(vector<string> comments) const {
        if (comments == _comments_or_null || comments.empty()) {
            return shared_from_this();
        } else {
            // Don't re-use with_comments, because we've already checked whether they're equal.
            // If they're not equal now, the concatenated comments won't be equal either.
            comments.insert(comments.begin(), _comments_or_null.begin(), _comments_or_null.end());
            return make_shared<simple_config_origin>(_description, _line_number, _line_number, _origin_type,
                                                     _resource_or_null, move(comments));
        }
    }

    shared_ptr<const simple_config_origin> simple_config_origin::prepend_comments(vector<string> comments) const {
        if (comments == _comments_or_null || comments.empty()) {
            return shared_from_this();
        } else {
            // Don't re-use with_comments, because we've already checked whether they're equal.
            // If they're not equal now, the concatenated comments won't be equal either.
            comments.insert(comments.end(), _comments_or_null.begin(), _comments_or_null.end());
            return make_shared<simple_config_origin>(_description, _line_number, _line_number, _origin_type,
                                                     _resource_or_null, move(comments));
        }
    }

    shared_origin simple_config_origin::merge_origins(shared_origin a, shared_origin b) {
        return merge_two(dynamic_pointer_cast<const simple_config_origin>(a), dynamic_pointer_cast<const simple_config_origin>(b));
    }

    shared_origin simple_config_origin::merge_origins(std::vector<shared_value> const& stack) {
        vector<shared_origin> origins;
        origins.reserve(stack.size());

        for (auto& v : stack) {
            origins.push_back(v->origin());
        }

        return merge_origins(origins);
    }

    shared_origin simple_config_origin::merge_origins(std::vector<shared_origin> const& stack) {
        if (stack.empty()) {
            throw config_exception(_("can't merge empty list of origins"));
        } else if (stack.size() == 1) {
            return stack.front();
        } else if (stack.size() == 2) {
            return(merge_two(dynamic_pointer_cast<const simple_config_origin>(stack[0]),
                             dynamic_pointer_cast<const simple_config_origin>(stack[1])));
        } else {
            vector<shared_origin> remaining;
            for (auto& o : stack) {
                remaining.push_back(dynamic_pointer_cast<const simple_config_origin>(o));
            }
            while (remaining.size() > 2) {
                auto c = dynamic_pointer_cast<const simple_config_origin>(remaining.back());
                remaining.pop_back();
                auto b = dynamic_pointer_cast<const simple_config_origin>(remaining.back());
                remaining.pop_back();
                auto a = dynamic_pointer_cast<const simple_config_origin>(remaining.back());
                remaining.pop_back();

                shared_origin merged = merge_three(a, b, c);
                remaining.push_back(merged);
            }

            //  should be down to either 1 or 2
            return merge_origins(remaining);
        }
    }

    shared_ptr<const simple_config_origin> simple_config_origin::merge_two(shared_ptr<const simple_config_origin> a,
                                                                           shared_ptr<const simple_config_origin> b) {
        string merged_desc;
        int merged_start_line;
        int merged_end_line;

        origin_type merged_type;
        if (a->_origin_type == b->_origin_type) {
            merged_type = a->_origin_type;
        } else {
            merged_type = origin_type::GENERIC;
        }

        static const string MERGE_OF_PREFIX { "merge of " };
        // first use the "description" field which has no line numbers
        // cluttering it.
        string a_desc { a->description() };
        string b_desc { b->description() };
        if (boost::starts_with(a_desc, MERGE_OF_PREFIX)) {
            a_desc = a_desc.substr(MERGE_OF_PREFIX.length());
        }

        if (boost::starts_with(b_desc, MERGE_OF_PREFIX)) {
            b_desc = b_desc.substr(MERGE_OF_PREFIX.length());
        }

        if (a_desc == b_desc) {
            merged_desc = a_desc;

            if (a->line_number() < 0) {
                merged_start_line = b->line_number();
            } else if (b->line_number() < 0) {
                merged_start_line = a->line_number();
            } else {
                merged_start_line = min(a->_end_line_number, b->_end_line_number);
            }

            merged_end_line = max(a->_end_line_number, b->_end_line_number);
        } else {
            // this whole merge song-and-dance was intended to avoid this case
            // whenever possible, but we've lost. Now we have to lose some
            // structured information and cram into a string.

            merged_desc = MERGE_OF_PREFIX + a_desc + "," + b_desc;
            merged_start_line = -1;
            merged_end_line = -1;
        }

        string merged_resource;
        if (a->_resource_or_null == b->_resource_or_null) {
            merged_resource = a->_resource_or_null;
        }

        vector<string> merged_comments;
        if (a->_comments_or_null == b->_comments_or_null) {
            merged_comments = a->_comments_or_null;
        } else {
            if (!a->_comments_or_null.empty())
            {
                merged_comments.insert(merged_comments.end(), a->_comments_or_null.begin(), a->_comments_or_null.end());
            }
            if (!b->_comments_or_null.empty())
            {
                merged_comments.insert(merged_comments.end(), b->_comments_or_null.begin(), b->_comments_or_null.end());
            }
        }

        return make_shared<simple_config_origin>(move(merged_desc), move(merged_start_line), move(merged_end_line), move(merged_type),
                                                 move(merged_resource), move(merged_comments));
    }

    shared_ptr<const simple_config_origin> simple_config_origin::merge_three(shared_ptr<const simple_config_origin> a,
                                                                             shared_ptr<const simple_config_origin> b,
                                                                             shared_ptr<const simple_config_origin> c) {
        if (similarity(a, b) >= similarity(b, c)) {
            return merge_two(merge_two(a, b), c);
        } else {
            return merge_two(a, merge_two(b, c));
        }
    }

    int simple_config_origin::similarity(shared_ptr<const simple_config_origin> a,
                                         shared_ptr<const simple_config_origin> b) {
        int count = 0;

        if (a->_origin_type == b->_origin_type) {
            count += 1;
        }
        if (a->_description == b->_description) {
            count += 1;
        }

        // only count these if the description field (which is the file
        // or resource name) also matches.
        if (a->_line_number == b->_line_number) {
            count += 1;
        }
        if (a->_end_line_number == b->_end_line_number) {
            count += 1;
        }
        if (a->_resource_or_null == b->_resource_or_null) {
            count += 1;
        }

        return count;
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
