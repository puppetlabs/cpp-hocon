#include <internal/config_exception.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>

namespace hocon {

    bool resolve_context::is_restricted_to_child() const
    {
        // TODO: implement
        throw config_exception("resolve_context::is_restricted_to_child is not yet implemented");
    }

    config_resolve_options resolve_context::options() const
    {
        return _options;
    }

    resolve_result<shared_value> resolve_context::resolve(shared_value original, resolve_source const& source) const
    {
        // TODO: implement
        throw config_exception("resolve_context::resolve is not yet implemented");
    }

    path resolve_context::restrict_to_child() const {
        return _restrict_to_child;
    }

    resolve_context resolve_context::restrict(path restrict_to) const {
        // TODO: implement
        throw config_exception("resolve_context::restrict is not yet implemented");
    }

    resolve_context resolve_context::unrestricted() const {
        // TODO: implement
        throw config_exception("resolve_context::unrestricted is not yet implemented");
    }

}  // namespace hocon
