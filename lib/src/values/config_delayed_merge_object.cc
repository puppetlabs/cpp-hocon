#include <internal/values/config_delayed_merge_object.hpp>
#include <internal/values/config_delayed_merge.hpp>
#include <internal/values/simple_config_list.hpp>
#include <hocon/config_exception.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_delayed_merge_object::config_delayed_merge_object(shared_origin origin, vector<shared_value> const &stack) :
        config_object(move(origin)), _stack(move(stack)) {
        if (_stack.empty()) {
            throw config_exception(_("creating empty delayed merge object"));
        }

        if (!dynamic_pointer_cast<const config_object>(_stack.front())) {
            throw config_exception(_("created a delayed merge object not guaranteed to be an object"));
        }

        for (auto& v : _stack) {
            if (dynamic_pointer_cast<const config_delayed_merge>(v) || dynamic_pointer_cast<const config_delayed_merge_object>(v)) {
                throw config_exception(_("placed nested delayed_merge in a config_delayed_merge_object, should have consolidated stack"));
            }
        }
    }

    shared_value config_delayed_merge_object::make_replacement(resolve_context const &context, int skipping) const {
        return config_delayed_merge::make_replacement(move(context), _stack, move(skipping));
    }

    shared_object config_delayed_merge_object::with_value(path raw_path, shared_value value) const {
        throw not_resolved();
    }

    shared_object config_delayed_merge_object::with_value(string key, shared_value value) const {
        throw not_resolved();
    }

    shared_object config_delayed_merge_object::new_copy(resolve_status const& status, shared_origin origin) const {
        if (status != get_resolve_status()) {
            throw bug_or_broken_exception(_("attempt to create resolved config_delayted_merge_object"));
        }
        return make_shared<config_delayed_merge_object>(move(origin), _stack);
    }

    unwrapped_value config_delayed_merge_object::unwrapped() const {
        throw config_exception(_("need to config::resolve before using this object, see the API docs."));
    }

    shared_value config_delayed_merge_object::attempt_peek_with_partial_resolve(string const& key) const {
        /* a partial resolve of a ConfigDelayedMergeObject always results in a
         * SimpleConfigObject because all the substitutions in the stack get
         * resolved in order to look up the partial.
         * So we know here that we have not been resolved at all even
         * partially.
         * Given that, all this code is probably gratuitous, since the app code
         * is likely broken. But in general we only throw NotResolved if you try
         * to touch the exact key that isn't resolved, so this is in that
         * spirit
         */

        // we'll be able to return a key if we have a value that ignores
        // fallbacks, prior to any unmergeable values.
        for (auto&& layer : _stack) {
            if (auto object_layer = dynamic_pointer_cast<const config_object>(layer)) {
                auto v = object_layer->attempt_peek_with_partial_resolve(key);
                if (v != nullptr) {
                    if (v->ignores_fallbacks()) {
                        // we know we won't need to merge anything in to this value
                        return v;
                    } else {
                        /* we can't return this value because we know there are
                         * unmergeable values later in the stack that may
                         * contain values that need to be merged with this
                         * value. we'll throw the exception when we get to those
                         * unmergeable values, so continue here.
                         */
                        continue;
                    }
                } else if (dynamic_pointer_cast<const unmergeable>(layer)) {
                    /* an unmergeable object (which would be another
                     * config_delayed_merge_object) can't know that a key is
                     * missing, so it can't return null; it can only return a
                     * value or throw not_possible_to_resolve
                     */
                    throw bug_or_broken_exception(_("should not be reached: unmergeable object returned null value"));
                } else {
                    /* a non-unmergeable config_objeect that returned null
                     * for the key in question is not relevant, we can keep
                     * looking for a value.
                     */
                    continue;
                }
            } else if (dynamic_pointer_cast<const unmergeable>(layer)) {
                throw not_resolved_exception(_("Key '{1}' is not available at '{2}' because value at '{3}' has not been resolved and may turn out to contain or hide '{4}'. Be sure to config::resolve() before using a config object", key, origin()->description(), layer->origin()->description(), key));
            } else if (layer->get_resolve_status() == resolve_status::UNRESOLVED) {
                /* if the layer is not an object, and not a substitution or
                 * merge, then it's something that's unresolved because it _contains_
                 * an unresolved object... i.e. it's an array
                 */
                if (!dynamic_pointer_cast<const simple_config_list>(layer)) {
                    throw bug_or_broken_exception(_("Expecting a list here, not {1}", layer->render()));
                    // all later objects will be hidden so we can say we won't find the key
                    return nullptr;
                }
            } else {
                /* non-object, but resolved, like an integer or something.
                 * has no children so the one we're after won't be in it.
                 * we would only have this in the stack in case something
                 * else "looks back" to it due to a cycle.
                 * anyway at this point we know we can't find the key anymore.
                 */
                if (!layer->ignores_fallbacks()) {
                    throw bug_or_broken_exception(_("resolved non-object should ignore fallbacks"));
                }
                return nullptr;
            }
        }
        /* If we get here, then we never found anything unresolved which means
         * the ConfigDelayedMergeObject should not have existed. some
         * invariant was violated.
         */
        throw bug_or_broken_exception(_("Delayed merge stack does not contain any unmergeable values"));
    }

    unordered_map<string, shared_value> const& config_delayed_merge_object::entry_set() const {
        throw not_resolved();
    }

    shared_object config_delayed_merge_object::without_path(path raw_path) const {
        throw not_resolved();
    }

    shared_object config_delayed_merge_object::with_only_path(path raw_path) const {
        throw not_resolved();
    }

    shared_object config_delayed_merge_object::with_only_path_or_null(path raw_path) const {
        throw not_resolved();
    }

    bool config_delayed_merge_object::operator==(config_value const& other) const {
        return equals<config_delayed_merge_object>(other, [&](config_delayed_merge_object const& o) { return _stack == o._stack; });
    }

    bool config_delayed_merge_object::ignores_fallbacks() const {
        return _stack.back()->ignores_fallbacks();
    }

    not_resolved_exception config_delayed_merge_object::not_resolved() const {
        return not_resolved_exception(_("need to config::resolve() before using this object, see the API docs for config::resolve()"));
    }

    shared_value config_delayed_merge_object::replace_child(shared_value const& child, shared_value replacement) const
    {
        auto new_stack = replace_child_in_list(_stack, child, move(replacement));
        if (new_stack.empty()) {
             return nullptr;
        } else {
            return make_shared<config_delayed_merge>(origin(), new_stack);
        }
    }

    bool config_delayed_merge_object::has_descendant(shared_value const& descendant) const
    {
        return has_descendant_in_list(_stack, descendant);
    }

    void config_delayed_merge_object::render(string& s, int indent, bool at_root, string const& at_key, config_render_options options) const {
        config_delayed_merge::render(_stack, s, indent, at_root, at_key, options);
    }

    void config_delayed_merge_object::render(string& s, int indent, bool at_root, config_render_options options) const {
        render(s, indent, at_root, "", options);
    }
}  // namespace hocon
