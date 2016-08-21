#include <internal/values/config_reference.hpp>
#include <hocon/config_exception.hpp>
#include <hocon/config_object.hpp>
#include <internal/resolve_source.hpp>
#include <internal/container.hpp>
#include <internal/substitution_expression.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_reference::config_reference(shared_origin origin, std::shared_ptr<substitution_expression> expr, int prefix_length)
            : config_value(origin), _expr(expr), _prefix_length(prefix_length) { }

    config_reference::type config_reference::value_type() const {
        throw not_resolved_exception(_("ur lame"));
    }

    vector<shared_value> config_reference::unmerged_values() const {
        return {shared_from_this()};
    }

    unwrapped_value config_reference::unwrapped() const {
        throw not_resolved_exception(_("Can't unwrap a config reference."));
    }

    shared_value config_reference::new_copy(shared_origin origin) const {
        return make_shared<config_reference>(origin, _expr, _prefix_length);
    }

    shared_ptr<substitution_expression> config_reference::expression() const {
        return _expr;
    }

    bool config_reference::operator==(config_value const &other) const {
        return equals<config_reference>(other, [&](config_reference const& o) { return *_expr == *o._expr; });
    }

    resolve_status config_reference::get_resolve_status() const {
        return resolve_status::UNRESOLVED;
    }

    resolve_result<shared_value> config_reference::resolve_substitutions(resolve_context const &context, resolve_source const &source) const {
        resolve_context new_context = context.add_cycle_marker(shared_from_this());
        shared_value v;

        try {
            auto result_with_path = source.lookup_subst(move(new_context), _expr, _prefix_length);
            new_context = result_with_path.result.context;

            if (result_with_path.result.value) {
                resolve_source recursive_resolve_source {dynamic_pointer_cast<const config_object>(result_with_path.path_from_root.back()), result_with_path.path_from_root};
                auto result = new_context.resolve(result_with_path.result.value, recursive_resolve_source);
                v = result.value;
                new_context = result.context;
            } else {
                v = nullptr;
            }
        } catch (not_possible_to_resolve_exception& ex) {
            if (_expr->optional()) {
                v = nullptr;
            } else {
                throw config_exception(_("{1} was part of a cycle of substitutions.", _expr->to_string()));
            }
        }

        if (!v && !_expr->optional()) {
            if (new_context.options().get_allow_unresolved()) {
                return make_resolve_result(new_context.remove_cycle_marker(shared_from_this()), shared_from_this());
            } else {
                throw unresolved_substitution_exception(*origin(), _expr->to_string());
            }
        } else {
            return make_resolve_result(new_context.remove_cycle_marker(shared_from_this()), v);
        }
    }

    void config_reference::render(std::string& s, int indent, bool at_root, config_render_options options) const {
        s += _expr->to_string();
    }
}
