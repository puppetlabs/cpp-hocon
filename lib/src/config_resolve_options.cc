#include <hocon/config_resolve_options.hpp>

namespace hocon {

    config_resolve_options::config_resolve_options(bool use_system_environment, bool allow_unresolved) :
        _use_system_environment(use_system_environment), _allow_unresovled(allow_unresolved) { }

    config_resolve_options config_resolve_options::set_use_system_environment(bool value) const {
        return config_resolve_options(value, _allow_unresovled);
    }

    bool config_resolve_options::get_use_system_environment() const {
        return _use_system_environment;
    }

    config_resolve_options config_resolve_options::set_allow_unresolved(bool value) const {
        return config_resolve_options(_use_system_environment, value);
    }

    bool config_resolve_options::get_allow_unresolved() const {
        return _allow_unresovled;
    }

}  // namespace hocon
