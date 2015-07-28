#include <internal/config_number.hpp>

using namespace std;

namespace hocon {

    config_number::config_number(simple_config_origin origin, string original_text) :
            abstract_config_value(origin), _original_text(original_text) { }

    config_value_type config_number::value_type() {
        return config_value_type::NUMBER;
    }

    string config_number::transform_to_string(){
        return _original_text;
    }

    bool config_number::is_whole() const {
        long as_long = long_value();
        return as_long == double_value();
    }

    bool config_number::operator==(const config_number &other) const {
        if(is_whole()) {
            return other.is_whole() && long_value() == other.long_value();
        } else {
            return !other.is_whole() && double_value() == other.double_value();
        }
    }
}  //namespace hocon
