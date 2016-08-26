#include <internal/default_transformer.hpp>
#include <hocon/config_value.hpp>
#include <boost/lexical_cast.hpp>
#include <internal/values/config_long.hpp>
#include <internal/values/config_double.hpp>
#include <internal/values/config_null.hpp>
#include <internal/values/config_boolean.hpp>
#include <internal/values/config_string.hpp>
#include <hocon/config_object.hpp>
#include <hocon/config_exception.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    shared_value default_transformer::transform(shared_value value, config_value::type requested) {
        if (value->value_type() == config_value::type::STRING) {
            string s = value->transform_to_string();
            switch (requested) {
                case config_value::type::NUMBER:
                    try {
                        int64_t v = boost::lexical_cast<int64_t>(s);
                        return make_shared<config_long>(value->origin(), v, s);
                    } catch (boost::bad_lexical_cast &ex) {
                        // try double
                    }
                    try {
                        double v = boost::lexical_cast<double>(s);
                        return make_shared<config_double>(value->origin(), v, s);
                    } catch (boost::bad_lexical_cast &ex) {
                        // we don't have a number
                    }
                    break;
                case config_value::type::CONFIG_NULL:
                    if (s == "null") {
                        return make_shared<config_null>(value->origin());
                    }
                    break;
                case config_value::type::BOOLEAN:
                    if (s == "true" || s == "yes" || s == "on") {
                        return make_shared<config_boolean>(value->origin(), true);
                    } else if (s == "false" || s == "no" || s == "off") {
                        return make_shared<config_boolean>(value->origin(), false);
                    }
                    break;
                case config_value::type::LIST:
                    // can't go STRING to LIST automatically
                    break;
                case config_value::type::OBJECT:
                    // can't go STRING ot OBJECT automatically
                    break;
                case config_value::type::STRING:
                    // no-op, already a string
                    break;
                case config_value::type::UNSPECIFIED:
                    throw config_exception(_("No target value type specified"));
            }
        } else if (requested == config_value::type::STRING) {
            // if we converted null to string here, then you wouldn't properly get a missing value error
            // i you tried to ge a null value as a string
            switch (value->value_type()) {
                case config_value::type::NUMBER:
                case config_value::type::BOOLEAN:
                    return make_shared<config_string>(value->origin(), value->transform_to_string(),
                                                      config_string_type::QUOTED);
                case config_value::type::CONFIG_NULL:
                    // this method will throw instead of returning null as a string
                    break;
                case config_value::type::OBJECT:
                    // can't convert OBJECT to STRING automatically
                    break;
                case config_value::type::LIST:
                    // can't convert LIST to STRING automatically
                    break;
                case config_value::type::STRING:
                    // no-op, already a string
                    break;
                case config_value::type::UNSPECIFIED:
                    throw config_exception(_("No target value type specified"));
            }
        } else if (requested == config_value::type::LIST && value->value_type() == config_value::type::OBJECT) {
            // TODO: implement this later when we support complex config objects
            throw config_exception(_("We currently do not support lists"));
        }

        return value;
    }


}  // namespace hocon
