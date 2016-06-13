#include <internal/values/config_delayed_merge.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <hocon/config_exception.hpp>

#include <internal/resolve_context.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>

using namespace std;

namespace hocon {

    config_delayed_merge::config_delayed_merge(shared_origin origin, std::vector<shared_value> stack) :
        config_value(move(origin)), _stack(move(stack)) {
        if (_stack.empty()) {
            throw config_exception("creating empty delayed merge value");
        }

        for (auto& v : stack) {
            if (dynamic_pointer_cast<const config_delayed_merge>(v) || dynamic_pointer_cast<const config_delayed_merge_object>(v)) {
                throw config_exception("placed nested delayed_merge in a config_delayed_merge, should have consolidated stack");
            }
        }
    }

    shared_value config_delayed_merge::make_replacement(resolve_context const &context, int skipping) const {
        return config_delayed_merge::make_replacement(move(context), _stack, move(skipping));
    }

    shared_value config_delayed_merge::make_replacement(resolve_context const &context,
                                                        std::vector<shared_value> stack,
                                                        int skipping) {
        vector<shared_value> sub_stack(stack.begin() + skipping, stack.end());

        if (sub_stack.empty()) {
            return nullptr;
        } else {
            // generate a new merge stack from only the remaining items
            shared_value merged = nullptr;
            for (auto&& v : sub_stack) {
                if (merged == nullptr) {
                    merged = v;
                } else {
                    merged = dynamic_pointer_cast<const config_value>(merged->with_fallback(v));
                }
            }
            return merged;
        }
    }

    config_value::type config_delayed_merge::value_type() const {
        throw config_exception("called value_type() on value with unresolved substitutions, need to config#resolve() first, see API docs.");
    }

    unwrapped_value config_delayed_merge::unwrapped() const {
        throw config_exception("called unwrapped() on value with unresolved substitutions, need to config::resolve() first, see API docs.");
    }

    vector<shared_value> config_delayed_merge::unmerged_values() const {
        return _stack;
    }

    resolve_result<shared_value> config_delayed_merge::resolve_substitutions(resolve_context const& context, resolve_source const& source) const {
        return resolve_substitutions(dynamic_pointer_cast<const replaceable_merge_stack>(shared_from_this()), _stack, context, source);
    }

    resolve_result<shared_value> config_delayed_merge::resolve_substitutions(shared_ptr<const replaceable_merge_stack> replaceable, vector<shared_value> const& stack, resolve_context const& context, resolve_source const& source) {
        // TODO add tracing/logging

        resolve_context new_context = context;
        int count = 0;
        shared_value merged;

        for (const auto& end : stack) {
            resolve_source source_for_end = source;

            if (dynamic_pointer_cast<const replaceable_merge_stack>(end)) {
                throw bug_or_broken_exception("A delayed merge should not contain another one");
            } else if (dynamic_pointer_cast<const unmergeable>(end)) {
                shared_value remainder = replaceable->make_replacement(context, count+1);

                // TODO more tracing

                source_for_end = source.replace_within_current_parent(dynamic_pointer_cast<const config_value>(replaceable), remainder);

                // TODO more tracing

                source_for_end = source_for_end.reset_parents();
            } else {
                source_for_end = source.push_parent(replaceable);
            }

            // TODO tracing

            auto result = new_context.resolve(end, source_for_end);
            auto resolved_end = result.value;
            new_context = result.context;

            if (resolved_end) {
                if (!merged) {
                    merged = resolved_end;
                } else {
                    // TODO tracing
                    merged = dynamic_pointer_cast<const config_value>(merged->with_fallback(resolved_end));
                }
            }

            count++;

            // TOOD tracing
        }
        return make_resolve_result(new_context, merged);
    }

    shared_value config_delayed_merge::new_copy(shared_origin origin) const {
        return make_shared<config_delayed_merge>(move(origin), _stack);
    }

    bool config_delayed_merge::operator==(config_value const& other) const {
        return equals<config_delayed_merge>(other, [&](config_delayed_merge const& o) { return _stack == o._stack; });
    }

    shared_value config_delayed_merge::replace_child(shared_value const& child, shared_value replacement) const
    {
        auto new_stack = replace_child_in_list(_stack, child, move(replacement));
        if (new_stack.empty()) {
             return nullptr;
        } else {
            return make_shared<config_delayed_merge>(origin(), new_stack);
        }
    }

    bool config_delayed_merge::has_descendant(shared_value const& descendant) const
    {
        return has_descendant_in_list(_stack, descendant);
    }


    bool config_delayed_merge::ignores_fallbacks() const {
        return _stack.back()->ignores_fallbacks();
    }

}  // namespace hocon::config_delayed_merge
