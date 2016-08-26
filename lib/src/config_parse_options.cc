#include <hocon/config_parse_options.hpp>
#include <hocon/config_includer.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_parse_options::config_parse_options(shared_string origin_desc,
            bool allow_missing, shared_includer includer, config_syntax syntax) :
        _syntax(syntax), _origin_description(move(origin_desc)),
        _allow_missing(allow_missing), _includer(move(includer)) {}

    config_parse_options::config_parse_options(): config_parse_options(nullptr, true, nullptr, config_syntax::CONF) {}

    config_parse_options config_parse_options::defaults() {
        return config_parse_options{nullptr, true, nullptr, config_syntax::UNSPECIFIED };
    }

    config_parse_options config_parse_options::set_syntax(config_syntax syntax) const
    {
        return config_parse_options{_origin_description, _allow_missing, _includer, syntax};
    }

    config_syntax const& config_parse_options::get_syntax() const
    {
        return _syntax;
    }

    config_parse_options config_parse_options::set_origin_description(shared_string origin_description) const
    {
        return config_parse_options{move(origin_description), _allow_missing, _includer, _syntax};
    }


    shared_string const& config_parse_options::get_origin_description() const
    {
        return _origin_description;
    }

    config_parse_options config_parse_options::with_fallback_origin_description(shared_string origin_description) const
    {
        if (!_origin_description) {
            return set_origin_description(origin_description);
        } else {
            return *this;
        }
    }

    config_parse_options config_parse_options::set_allow_missing(bool allow_missing) const
    {
        return config_parse_options{_origin_description, allow_missing, _includer, _syntax};
    }

    bool config_parse_options::get_allow_missing() const
    {
        return _allow_missing;
    }

    config_parse_options config_parse_options::set_includer(shared_includer includer) const
    {
        return config_parse_options{ _origin_description, _allow_missing, move(includer), _syntax};
    }

    config_parse_options config_parse_options::prepend_includer(shared_includer includer) const
    {
        if (!includer) {
            throw runtime_error(_("null includer passed to prepend_includer"));
        }
        if (_includer == includer) {
            return *this;
        } else if (_includer) {
            return set_includer(includer->with_fallback(_includer));
        } else {
            return set_includer(includer);
        }
    }

    config_parse_options config_parse_options::append_includer(shared_includer includer) const
    {
        if (!includer) {
            throw runtime_error(_("null includer passed to append_includer"));
        }
        if (_includer == includer) {
            return *this;
        } else if (_includer) {
            return set_includer(_includer->with_fallback(move(includer)));
        } else {
            return set_includer(includer);
        }
    }

    shared_includer const& config_parse_options::get_includer() const
    {
        return _includer;
    }

}  // namespace hocon
