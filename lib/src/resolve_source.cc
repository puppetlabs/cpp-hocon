#include <internal/resolve_source.hpp>
#include <hocon/config_exception.hpp>
#include <internal/substitution_expression.hpp>
#include <hocon/config_value.hpp>
#include <hocon/config_object.hpp>
#include <internal/container.hpp>

namespace hocon {

    using namespace std;

    resolve_source::resolve_source(shared_object root)
         : _root(root) { }

    resolve_source::resolve_source(shared_object root, node path_from_root)
        : _root(root), _path_from_root(path_from_root) { }

    resolve_source resolve_source::push_parent(shared_container parent) const
    {
        if (!parent) {
            throw bug_or_broken_exception("can't push null parent");
        }

        if ( _path_from_root.empty() ) {
            if (dynamic_pointer_cast<const config_object>(parent) == _root) {
                return resolve_source(_root, {parent});
            } else {
                return *this;
            }
        } else {
            auto new_path_from_root = _path_from_root;
            new_path_from_root.push_front(parent);
            return resolve_source(_root, new_path_from_root);
        }
    }

    resolve_source::result_with_path resolve_source::lookup_subst(resolve_context context,
                                                                  std::shared_ptr<substitution_expression> subst,
                                                                  int prefix_length) const {
        auto result = find_in_object(_root, context, subst->get_path());

        if (!result.result.value) {
            auto unprefixed = subst->get_path().sub_path(prefix_length);

            if (prefix_length > 0) {
                result = find_in_object(_root, result.result.context, unprefixed);
            }

            if (!result.result.value && result.result.context.options().get_use_system_environment()) {
                // TODO: implement this o:
                throw config_exception("lookup in system environment is not implemented");
            }
        }

        return result;
    }

    resolve_source::result_with_path resolve_source::find_in_object(shared_object obj, resolve_context context,
                                                                    path the_path) const {
        auto restriction = context.restrict_to_child();
        auto partially_resolved = context.restrict(the_path).resolve(dynamic_pointer_cast<const config_value>(obj), {obj});
        auto new_context = partially_resolved.context.restrict(restriction);

        if (auto value = dynamic_pointer_cast<const config_object>(partially_resolved.value)) {
            value_with_path pair = find_in_object(value, the_path);
            return {make_resolve_result(new_context, pair.value), pair.path_from_root};

        } else {
            throw bug_or_broken_exception("resolved object to non-object");
        }
    }

    resolve_source::value_with_path resolve_source::find_in_object(shared_object obj, path the_path) const {
        // TODO: implement improve_not_resolved
        return find_in_object(obj, the_path, {});
    }

    resolve_source::value_with_path resolve_source::find_in_object(shared_object obj, path the_path, node parents) const {
        auto key = the_path.first();
        auto next = the_path.remainder();

        auto v = obj->attempt_peek_with_partial_resolve(*key);

        parents.push_front(dynamic_pointer_cast<const container>(obj));

        if (next.empty()) {
            return {v, parents};
        } else {
            if (auto value = dynamic_pointer_cast<const config_object>(v)) {
                return find_in_object(value, next, parents);
            } else {
                return {nullptr, parents};
            }
        }
    }
}  // namespace hocon
