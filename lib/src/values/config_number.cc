#include <internal/values/config_number.hpp>
#include <internal/values/config_int.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_double.hpp>
#include <hocon/config_exception.hpp>
#include <leatherman/locale/locale.hpp>
#include <limits>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_number::config_number(shared_origin origin, string original_text) :
            config_value(move(origin)), _original_text(move(original_text)) { }

    config_value::type config_number::value_type() const {
        return config_value::type::NUMBER;
    }

    string config_number::transform_to_string() const {
        return _original_text;
    }

    bool config_number::is_whole() const {
        long as_long = long_value();
        return as_long == double_value();
    }

    bool config_number::operator==(const config_number &other) const {
        if (is_whole()) {
            return other.is_whole() && long_value() == other.long_value();
        } else {
            return !other.is_whole() && double_value() == other.double_value();
        }
    }

    bool config_number::operator!=(const config_number &other) const {
        return !(*this == other);
    }

    int config_number::int_value_range_checked(std::string const& path) const {
        long l = long_value();
        if (l < numeric_limits<int>::min() || l > numeric_limits<int>::max()) {
            throw config_exception(_("Tried to get int from out of range value {1}", to_string(l)));
        }
        return static_cast<int>(l);
    }

    shared_ptr<config_number> config_number::new_number(
            shared_origin origin, int64_t value, std::string original_text) {
        if (value >= numeric_limits<int>::min() && value <= numeric_limits<int>::max()) {
            return make_shared<config_int>(move(origin), static_cast<int>(value),
                                                         move(original_text));
        } else {
            return make_shared<config_long>(move(origin), value, move(original_text));
        }
    }

    shared_ptr<config_number> config_number::new_number(
            shared_origin origin, double value, std::string original_text) {
        int64_t as_long = static_cast<int64_t>(value);
        if (as_long == value) {
            return new_number(move(origin), as_long, move(original_text));
        } else {
            return unique_ptr<config_double>(new config_double(move(origin), value, move(original_text)));
        }
    }

    bool config_number::operator==(config_value const& other) const {
        return equals<config_number>(other, [&](config_number const& o) {
            return long_value() == o.long_value() && double_value() == o.double_value();
        });
    }

}  // namespace hocon
