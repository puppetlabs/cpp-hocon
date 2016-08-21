#include <internal/values/config_delayed_merge.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <hocon/config_exception.hpp>

#include <internal/resolve_context.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>

#include <leatherman/locale/locale.hpp>

#include <algorithm>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_delayed_merge::config_delayed_merge(shared_origin origin, std::vector<shared_value> stack) :
        config_value(move(origin)), _stack(move(stack)) {
        if (_stack.empty()) {
            throw config_exception(_("creating empty delayed merge value"));
        }

        for (auto& v : stack) {
            if (dynamic_pointer_cast<const config_delayed_merge>(v) || dynamic_pointer_cast<const config_delayed_merge_object>(v)) {
                throw config_exception(_("placed nested delayed_merge in a config_delayed_merge, should have consolidated stack"));
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
        throw config_exception(_("called value_type() on value with unresolved substitutions, need to config#resolve() first, see API docs."));
    }

    unwrapped_value config_delayed_merge::unwrapped() const {
        throw config_exception(_("called unwrapped() on value with unresolved substitutions, need to config::resolve() first, see API docs."));
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
                throw bug_or_broken_exception(_("A delayed merge should not contain another one"));
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

    void config_delayed_merge::render(string& s, int indent, bool at_root, string const& at_key, config_render_options options) const {
        render(_stack, s, indent, at_root, at_key, options);
    }

    void config_delayed_merge::render(string& s, int indent, bool at_root, config_render_options options) const {
        render(s, indent, at_root, "", options);
    }

    // static method also used by config_delayed_merge_object
    void config_delayed_merge::render(vector<shared_value> const& stack, string& s, int indent_value, bool at_root, string const& at_key, config_render_options options) {
        bool comment_merge = options.get_comments();

        if (comment_merge) {
            s += _("# unresolved merge of {1} values follows (\n", to_string(stack.size()));
            if (at_key == "") {
                indent(s, indent_value, options);
                s += _("# this unresolved merge will not be parseable because it's at the root of the object =\n");
                indent(s, indent_value, options);
                s += _("# the HOCON format has no way to list multiple root objects in a single file \n");
            }
        }

        vector<shared_value> reversed;
        reversed.insert(reversed.end(), stack.begin(), stack.end());
        reverse(reversed.begin(), reversed.end());

        int i = 0;
        for (auto const& v : reversed) {
            if (comment_merge) {
                indent(s, indent_value, options);
                if (at_key == "") {
                    s += _("#     unmerged value {1} for key {2} from ", to_string(i), hocon::render_json_string(at_key));
                } else {
                    s += _("#     unmerged value {1} from ", to_string(i));
                }
                i++;
                s += v->origin()->description();
                s += "\n";

                for (string const& comment : v->origin()->comments()) {
                    indent(s, indent_value, options);
                    s += "# ";
                    s += comment;
                    s += "\n";
                }
            }
            indent(s, indent_value, options);

            if (at_key != "") {
                s += hocon::render_json_string(at_key);
                if (options.get_formatted()) {
                    s += " : ";
                } else {
                    s += ":";
                }
            }
            v->render(s, indent_value, at_root, options);
            s += ",";
            if (options.get_formatted()) {
                s += "\n";
            }
        }
        // chomp comma or newline
        s = s.substr(0, s.size() - 1);
        if (options.get_formatted()) {
            s = s.substr(0, s.size() - 1);  // also chomp comma
            s += "\n";
        }
        if (comment_merge) {
            indent(s, indent_value, options);
            s += _("# ) end of unresolved merge\n");
        }
    }
}  // namespace hocon::config_delayed_merge
