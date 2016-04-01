#include <internal/substitution_expression.hpp>

using namespace std;

namespace hocon {

    substitution_expression::substitution_expression(path the_path, bool optional) :
        _path(move(the_path)), _optional(move(optional)) { }

    path substitution_expression::get_path() const {
        return _path;
    }

    bool substitution_expression::optional() const {
        return _optional;
    }

    shared_ptr<substitution_expression> substitution_expression::change_path(path new_path) {
        if (new_path == _path) {
            return shared_from_this();
        } else {
            return make_shared<substitution_expression>(move(new_path), _optional);
        }
    }

    string substitution_expression::to_string() const {
        return string("${") + (_optional ? "?" : "") + _path.to_string() + "}";
    }

}  // namespace hocon

