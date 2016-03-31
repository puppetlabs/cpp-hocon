#pragma once

#include <memory>
#include <list>
#include <hocon/types.hpp>
#include <internal/resolve_result.hpp>

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
        };

        resolve_source(shared_object root);
        resolve_source(shared_object root, node path_from_root);
        resolve_source push_parent(std::shared_ptr<const container> parent) const;
        result_with_path lookup_subst(resolve_context context, std::shared_ptr<substitution_expression> subst, int prefix_length) const;

    private:
        struct value_with_path {
            shared_value value;
            node path_from_root;
        };

        shared_object _root;
        node _path_from_root;

        value_with_path find_in_object(shared_object obj, path the_path) const;
        result_with_path find_in_object(shared_object obj, resolve_context context, path the_path) const;
        value_with_path find_in_object(shared_object obj, path the_path, node parents) const;
    };
}  // namespace hocon
