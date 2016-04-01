#include <hocon/config_exception.hpp>
#include <hocon/config_value.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>

using namespace std;

namespace hocon {

    resolve_context::resolve_context(config_resolve_options options, path restrict_to_child)
         : _options(move(options)), _restrict_to_child(move(restrict_to_child)) { }

    bool resolve_context::is_restricted_to_child() const
    {
        return !_restrict_to_child.empty();
    }

    config_resolve_options resolve_context::options() const
    {
        return _options;
    }

    resolve_result<shared_value> resolve_context::resolve(shared_value original, resolve_source const& source) const
    {
        auto result = original->resolve_substitutions(*this, source);
        auto resolved = result.value;

        if (!resolved || resolved->get_resolve_status() == resolve_status::RESOLVED) {
            return result;
        }  else {
            throw config_exception("Partial resolves and incomplete resolutions are not yet implemented");
        }

        return result;
    }

    path resolve_context::restrict_to_child() const {
        return _restrict_to_child;
    }

    resolve_context resolve_context::restrict(path restrict_to) const {
        if (restrict_to == _restrict_to_child) {
            return *this;
        } else {
            return resolve_context(_options, restrict_to);
        }
    }

    resolve_context resolve_context::unrestricted() const {
        return restrict({});
    }

    shared_value resolve_context::resolve(shared_value value, shared_object root, config_resolve_options options) {
        resolve_source source { root };
        resolve_context context { options, path() };

        return context.resolve(value, source).value;
    }

}  // namespace hocon
