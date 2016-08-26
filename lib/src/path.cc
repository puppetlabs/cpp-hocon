#include <hocon/path.hpp>
#include <hocon/config_exception.hpp>
#include <internal/path_builder.hpp>
#include <internal/config_util.hpp>
#include <internal/path_parser.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    path::path() { }

    path::path(List<shared_string> other_path) : _path(move(other_path)) { }

    path::path(string first, path const& remainder) :
            _path(make_shared<string>(move(first)), remainder._path) { }

    path::path(vector<string> elements) {
        if (elements.size() == 0) {
            throw config_exception(_("Empty path"));
        }

        path remainder;
        if (elements.size() > 1) {
            path_builder builder;
            for (size_t i = 1; i < elements.size(); i++) {
                builder.append_key(elements[i]);
            }
            remainder = builder.result();
        }
        _path = List<shared_string>(make_shared<string>(move(elements[0])), remainder._path);
    }

    path::path(vector<path> paths_to_concat) {
        if (paths_to_concat.size() == 0) {
            throw config_exception(_("Empty path"));
        }

        path_builder builder;
        if (paths_to_concat[0].has_remainder()) {
            builder.append_path(paths_to_concat[0].remainder());
        }

        for (size_t i = 1; i < paths_to_concat.size(); i++) {
            builder.append_path(paths_to_concat[i]);
        }
        _path = List<shared_string>(paths_to_concat[0].first(), builder.result()._path);
    }

    shared_string path::first() const {
        if (_path.isEmpty()) {
            return nullptr;
        }
        return _path.front();
    }

    path path::remainder() const {
        if (_path.isEmpty()) {
            return path { };
        }
        return path(_path.popped_front());
    }

    bool path::has_remainder() const {
        return !remainder()._path.isEmpty();
    }

    shared_string path::last() const {
        path p = *this;
        while (p.has_remainder()) {
            p = p.remainder();
        }
        return p.first();
    }

    path path::prepend(path prefix) {
        path_builder builder;
        builder.append_path(prefix);
        builder.append_path(*this);
        return builder.result();
    }

    int path::length() const {
        int length = 1;
        path p = *this;
        while (p.has_remainder()) {
            length++;
            p = p.remainder();
        }
        return length;
    }

    path path::sub_path(int remove_from_front) {
        int count = remove_from_front;
        path p = *this;
        while (p.has_remainder() && count > 0) {
            count--;
            p = p.remainder();
        }
        return p;
    }

    path path::sub_path(int first_index, int last_index) {
        if (last_index < first_index) {
            throw config_exception(_("Bad call to sub_path: invalid range"));
        }

        path from = sub_path(first_index);
        path_builder builder;
        int count = last_index - first_index;
        while (count > 0) {
            count--;
            builder.append_key(*from.first());
            from = from.remainder();
            if (from.empty()) {
                throw config_exception(_("sub_path last_index out of range"));
            }
        }
        return builder.result();
    }

    bool path::empty() const {
        return _path.isEmpty();
    }

    bool path::starts_with(path other) const {
        path my_remainder = *this;
        path other_remainder = other;
        if (other_remainder.length() <= my_remainder.length()) {
            while (other_remainder.first()) {
                if (*other_remainder.first() != *my_remainder.first()) {
                    return false;
                }
                my_remainder = my_remainder.remainder();
                other_remainder = other_remainder.remainder();
            }
            return true;
        }
        return false;
    }

    path path::parent() const {
        if (!has_remainder()) {
            return path { };
        }

        path_builder builder;
        path p = *this;
        while (p.has_remainder()) {
            builder.append_key(*p.first());
            p = p.remainder();
        }
        return builder.result();
    }

    bool path::operator==(path const& other) const {
        if ((first() == nullptr) ^ (other.first() == nullptr)) {
            return false;
        }
        if (first() == nullptr && other.first() == nullptr) {
            return true;
        }
        return *first() == *other.first() &&
               remainder() == other.remainder();
    }

    bool path::operator!=(path const& other) const {
        return !(*this == other);
    }

    bool path::has_funky_chars(string const& s) {
        auto bad_char = find_if(s.begin(), s.end(), [] (char c) {
            return !(isalnum(c) || c == '-' || c == '_');
        });

        return bad_char != s.end();
    }

    void path::append_to_string(std::string &base) const {
        if (_path.isEmpty()) {
            return;
        }
        if (has_funky_chars(*first()) || first()->empty()) {
            base += render_json_string(*first());
        } else {
            base += *first();
        }
        if (has_remainder()) {
            base += ".";
            remainder().append_to_string(base);
        }
    }

    string path::to_string() const {
        string result = "Path(";
        append_to_string(result);
        result += ")";
        return result;
    }

    string path::render() const {
        string result = "";
        append_to_string(result);
        return result;
    }

    path path::new_key(string key) {
        return path(key, path { });
    }

    path path::new_path(string path_string) {
        return path_parser::parse_path(path_string);
    }

}  // namespace hocon
