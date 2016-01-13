#include <hocon/config_render_options.hpp>

namespace hocon {

    config_render_options::config_render_options(bool origin_comments, bool comments, bool formatted, bool json) :
        _origin_comments(origin_comments), _comments(comments), _formatted(formatted), _json(json) { }

    config_render_options config_render_options::concise() {
        return config_render_options(false, false, false, true);
    }

    config_render_options config_render_options::set_origin_comments(bool value) {
        return config_render_options(value, _comments, _formatted, _json);
    }

    bool config_render_options::get_origin_comments() const {
        return _origin_comments;
    }

    config_render_options config_render_options::set_comments(bool value) {
        return config_render_options(_origin_comments, value, _formatted, _json);
    }

    bool config_render_options::get_comments() const {
        return _comments;
    }

    config_render_options config_render_options::set_formatted(bool value) {
        return config_render_options(_origin_comments, _comments, value, _json);
    }

    bool config_render_options::get_formatted() const {
        return _formatted;
    }

    config_render_options config_render_options::set_json(bool value) {
        return config_render_options(_origin_comments, _comments, _formatted, value);
    }

    bool config_render_options::get_json() const {
        return _json;
    }

}  // namespace hocon
