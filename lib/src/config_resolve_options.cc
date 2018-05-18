#include <hocon/config_resolve_options.hpp>
#include <hocon/config_resolver.hpp>
#include <hocon/config_exception.hpp>

using leatherman::locale::_;

namespace hocon {

    const shared_resolver config_resolve_options::NULL_RESOLVER = make_shared<config_resolver>();

    config_resolve_options::config_resolve_options(bool use_system_environment, bool allow_unresolved, shared_resolver resolver) :
        _use_system_environment(use_system_environment), _allow_unresovled(allow_unresolved), _resolver(resolver) { }

    config_resolve_options config_resolve_options::set_use_system_environment(bool value) const {
        return config_resolve_options(value, _allow_unresovled, _resolver);
    }

    bool config_resolve_options::get_use_system_environment() const {
        return _use_system_environment;
    }

    config_resolve_options config_resolve_options::set_allow_unresolved(bool value) const {
        return config_resolve_options(_use_system_environment, value, _resolver);
    }

    bool config_resolve_options::get_allow_unresolved() const {
        return _allow_unresovled;
    }
    shared_resolver config_resolve_options::get_resolver() const {
        return _resolver;
    }

    config_resolve_options config_resolve_options::append_resolver(shared_resolver value) {
        if (value == nullptr) {
            throw bug_or_broken_exception(_("null resolver passed to append_resolver"));
        } else if (value == _resolver) {
            return *this;
        } else {
            return config_resolve_options(_use_system_environment, _allow_unresovled,
                                            _resolver->with_fallback(value));
        }
    }

}  // namespace hocon
