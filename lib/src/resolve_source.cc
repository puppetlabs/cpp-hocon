#include <internal/resolve_source.hpp>
#include <hocon/config_exception.hpp>
#include <internal/substitution_expression.hpp>
#include <hocon/config_value.hpp>
#include <hocon/config_object.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/container.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

namespace hocon {

    using namespace std;

    resolve_source::resolve_source(shared_object root)
         : _root(root) { }

    resolve_source::resolve_source(shared_object root, node path_from_root)
        : _root(root), _path_from_root(path_from_root) { }

    resolve_source resolve_source::push_parent(shared_container parent) const
    {
        if (!parent) {
            throw bug_or_broken_exception(_("can't push null parent"));
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

    resolve_source resolve_source::replace_current_parent(shared_ptr<const container> old, shared_ptr<const container> replacement) const
    {
        if (old == replacement) {
             return *this;
        } else if (! _path_from_root.empty()) {
            auto new_path = replace(_path_from_root, old, dynamic_pointer_cast<const config_value>(replacement));
            if (!new_path.empty()) {
                return resolve_source(dynamic_pointer_cast<const config_object>(new_path.back()), new_path);
            } else {
                return resolve_source(simple_config_object::empty());
            }
        } else {
            if (old == dynamic_pointer_cast<const container>(_root)) {
                return resolve_source(root_must_be_obj(replacement));
            } else {
                throw bug_or_broken_exception(_("attempt to replace root with invalid value"));
            }
        }
    }

    resolve_source resolve_source::replace_within_current_parent(shared_value old, shared_value replacement) const
    {
        if (old == replacement) {
             return *this;
        } else if (!_path_from_root.empty()) {
            auto parent = _path_from_root.front();
            auto new_parent = parent->replace_child(old, replacement);
            return replace_current_parent(parent, dynamic_pointer_cast<const container>(new_parent));
        } else {
            auto replacement_as_container = dynamic_pointer_cast<const container>(replacement);
            if (old == _root && replacement_as_container) {
                return resolve_source(root_must_be_obj(replacement_as_container));
            } else {
                throw bug_or_broken_exception(_("replace in parent not possible"));
            }
        }
    }

    resolve_source resolve_source::reset_parents() const
    {
        if (_path_from_root.empty()) {
            return *this;
        } else {
            return resolve_source(_root);
        }
    }

    resolve_source::result_with_path resolve_source::lookup_subst(resolve_context context,
                                                                  std::shared_ptr<substitution_expression> subst,
                                                                  int prefix_length) const {
        auto result = find_in_object(_root, move(context), subst->get_path());

        if (!result.result.value) {
            auto unprefixed = subst->get_path().sub_path(prefix_length);

            if (prefix_length > 0) {
                result = find_in_object(_root, result.result.context, unprefixed);
            }

            if (!result.result.value && result.result.context.options().get_use_system_environment()) {
                result = find_in_object(config::env_variables_as_config_object(), context, unprefixed);
            }
        }

        return result;
    }

    resolve_source::result_with_path resolve_source::find_in_object(shared_object obj, resolve_context context,
                                                                    path the_path) {
        auto restriction = context.restrict_to_child();
        auto partially_resolved = context.restrict(the_path).resolve(dynamic_pointer_cast<const config_value>(obj), {obj});
        auto new_context = partially_resolved.context.restrict(restriction);

        if (auto value = dynamic_pointer_cast<const config_object>(partially_resolved.value)) {
            value_with_path pair = find_in_object(value, the_path);
            return {make_resolve_result(new_context, pair.value), pair.path_from_root};

        } else {
            throw bug_or_broken_exception(_("resolved object to non-object"));
        }
    }

    not_resolved_exception resolve_source::improve_not_resolved(path what, not_resolved_exception const& original) {
        string new_message = _("{1} has not been resolved, you need to call config::resolve() see API docs for config::resolve()", what.render());
        if (new_message == original.what()) {
            return original;
        } else {
            return not_resolved_exception(new_message);
        }
    }

    resolve_source::value_with_path resolve_source::find_in_object(shared_object obj, path the_path) {
        try {
            return find_in_object(obj, the_path, {});
        } catch (const not_resolved_exception &e) {
            throw improve_not_resolved(move(the_path), e);
        }
    }

    resolve_source::value_with_path resolve_source::find_in_object(shared_object obj, path the_path, node parents) {
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

    shared_object resolve_source::root_must_be_obj(shared_ptr<const container> value) const {
        auto value_as_obj = dynamic_pointer_cast<const config_object>(value);
        if (value_as_obj) {
            return value_as_obj;
        } else {
            return simple_config_object::empty();
        }
    }

    resolve_source::node resolve_source::replace(const resolve_source::node& list, shared_ptr<const container> old, shared_value replacement)
    {
        auto child = list.front();
        if (child != old) {
            throw bug_or_broken_exception(_("Can only replace() the top node we're resolving"));
        }

        shared_ptr<const container> parent;
        if (list.size() > 1) {
            node copy(list);
            copy.pop_front();
            parent = copy.front();
        }

        auto replacement_as_container = dynamic_pointer_cast<const container>(replacement);
        if (!replacement || !replacement_as_container) {
            if (!parent) {
                return {};
            } else {
                auto new_parent = parent->replace_child(dynamic_pointer_cast<const config_value>(old), nullptr);
                node copy(list);
                copy.pop_front();
                return replace(copy, parent, new_parent);
            }
        } else {
            if (!parent) {
                return {replacement_as_container};
            } else {
                auto new_parent = parent->replace_child(dynamic_pointer_cast<const config_value>(old), replacement);
                node copy(list);
                copy.pop_front();
                auto new_tail = replace(copy, parent, new_parent);
                if (!new_tail.empty()) {
                    new_tail.push_front(replacement_as_container);
                    return new_tail;
                } else {
                    return {replacement_as_container};
                }
            }
        }
    }

    resolve_source::value_with_path::value_with_path(shared_value v, node path_from_root_value)
        : value(move(v)), path_from_root(move(path_from_root_value)) { }

    resolve_source::result_with_path::result_with_path(resolve_result<shared_value> result_value, node path_from_root_value)
        : result(move(result_value)), path_from_root(move(path_from_root_value)) { }
}  // namespace hocon
