#pragma once

#include <memory>
#include <list>
#include <hocon/types.hpp>
#include <internal/resolve_result.hpp>
#include <hocon/config_exception.hpp>

namespace hocon {

    class container;
    class resolve_context;
    class substitution_expression;

    class resolve_source {
    public:
        typedef std::list<std::shared_ptr<const container>> node;

        struct result_with_path {
            resolve_result<shared_value> result;
            node path_from_root;

            result_with_path(resolve_result<shared_value> result_value, node path_from_root_value);
        };

        resolve_source(shared_object root);
        resolve_source(shared_object root, node path_from_root);
        resolve_source push_parent(std::shared_ptr<const container> parent) const;
        result_with_path lookup_subst(resolve_context context, std::shared_ptr<substitution_expression> subst, int prefix_length) const;

        resolve_source replace_current_parent(std::shared_ptr<const container> old, std::shared_ptr<const container> replacement) const;
        resolve_source replace_within_current_parent(shared_value old, shared_value replacement) const;
        resolve_source reset_parents() const;

    private:
        struct value_with_path {
            shared_value value;
            node path_from_root;

            value_with_path(shared_value v, node path_from_root_value);
        };

        shared_object _root;
        node _path_from_root;

        static value_with_path find_in_object(shared_object obj, path the_path);
        static result_with_path find_in_object(shared_object obj, resolve_context context, path the_path);
        static value_with_path find_in_object(shared_object obj, path the_path, node parents);
        static not_resolved_exception improve_not_resolved(path what, not_resolved_exception const& original);

        shared_object root_must_be_obj(std::shared_ptr<const container> value) const;

        static node replace(const node& list, std::shared_ptr<const container> old, shared_value replacement);
    };
}  // namespace hocon
