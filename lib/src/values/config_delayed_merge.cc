#include <internal/values/config_delayed_merge.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <hocon/config_exception.hpp>

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
        throw config_exception("called value_type() on value with unresolved substitutions, need to config#resolve() first, see API docs");
    }

    vector<shared_value> config_delayed_merge::unmerged_values() const {
        return _stack;
    }

    shared_value config_delayed_merge::new_copy(shared_origin origin) const {
        return make_shared<config_delayed_merge>(move(origin), _stack);
    }

    bool config_delayed_merge::operator==(config_value const& other) const {
        return equals<config_delayed_merge>(other, [&](config_delayed_merge const& o) { return _stack == o._stack; });
    }

    bool config_delayed_merge::ignores_fallbacks() const {
        return _stack.back()->ignores_fallbacks();
    }

}  // namespace hocon::config_delayed_merge
