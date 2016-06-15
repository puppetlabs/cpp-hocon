#include <internal/values/config_string.hpp>

using namespace std;

namespace hocon {

    config_string::config_string(shared_origin origin, string text, config_string_type quoted) :
        config_value(move(origin)), _text(move(text)), _quoted(quoted) { }

    config_value::type config_string::value_type() const {
        return config_value::type::STRING;
    }

    string config_string::transform_to_string() const {
        return _text;
    }

    shared_value config_string::new_copy(shared_origin origin) const {
        return make_shared<config_string>(move(origin), _text, _quoted);
    }

    unwrapped_value config_string::unwrapped() const {
        return _text;
    }

    bool config_string::was_quoted() const {
        return _quoted == config_string_type::QUOTED;
    }

    bool config_string::operator==(config_value const& other) const {
        return equals<config_string>(other, [&](config_string const& o) { return _text == o._text; });
    }

    void config_string::render(std::string& s, int indent, bool at_root, config_render_options options) const {
        string rendered;

        if (options.get_json()) {
            rendered = hocon::render_json_string(_text);
        } else  {
            rendered = hocon::render_string_unquoted_if_possible(_text);
        }

        s += rendered;
    }


}  // namespace hocon
