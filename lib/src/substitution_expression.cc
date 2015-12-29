#include <internal/substitution_expression.hpp>

using namespace std;

namespace hocon {

    substitution_expression::substitution_expression(string path, bool optional) :
        _path(move(path)), _optional(move(optional)) { }

    string substitution_expression::path() const {
        return _path;
    }

    bool substitution_expression::optional() const {
        return _optional;
    }

    shared_ptr<substitution_expression> substitution_expression::change_path(std::string new_path) {
        if (new_path == _path) {
            return shared_from_this();
        } else {
            return make_shared<substitution_expression>(move(new_path), _optional);
        }
    }

    string substitution_expression::to_string() const {
        return string("${") + (_optional ? "?" : "") + _path + "}";
    }

}  // namespace hocon

