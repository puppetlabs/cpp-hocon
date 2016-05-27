#include <internal/values/simple_config_object.hpp>
#include <hocon/config_value.hpp>
#include <hocon/config_exception.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_source.hpp>
#include <internal/resolve_result.hpp>
#include <internal/container.hpp>

#include <algorithm>
#include <unordered_set>

using namespace std;

namespace hocon {

    struct simple_config_object::resolve_modifier : public modifier {
        resolve_modifier(resolve_context c, resolve_source s)
                : context(move(c)), source(move(s)), original_restrict(context.restrict_to_child()) {}

        shared_value modify_child_may_throw(string key, shared_value v) override
        {
            if (context.is_restricted_to_child()) {
                if (key == *context.restrict_to_child().first()) {
                    auto remainder = context.restrict_to_child().remainder();

                    if (remainder.empty()) {
                        auto result = context.restrict(remainder).resolve(v, source);
                        context = result.context.unrestricted().restrict(original_restrict);
                        return result.value;
                    } else {
                        return v;
                    }
                } else {
                    return v;
                }
            } else {
                auto result = context.unrestricted().resolve(v, source);
                context = result.context.unrestricted().restrict(original_restrict);
                return result.value;
            }
        }

        resolve_context context;
        resolve_source source;
        path original_restrict;
    };

    simple_config_object::simple_config_object(shared_origin origin,
                                               unordered_map <std::string, shared_value> value,
                                               resolve_status status, bool ignores_fallbacks) :
        config_object(move(origin)), _value(move(value)), _resolved(status), _ignores_fallbacks(ignores_fallbacks)
    {}

    simple_config_object::simple_config_object(shared_origin origin,
                                               unordered_map <std::string, shared_value> value)
         : config_object(move(origin)) {
        // These are in the body so I can call resolve_from_status
        // then move the value hash in a well-defined order.
        _resolved = resolve_status_from_value(value);
        _value = move(value);
        _ignores_fallbacks = false;
    }

    shared_value simple_config_object::attempt_peek_with_partial_resolve(std::string const& key) const {
        auto iter = _value.find(key);
        if (iter != _value.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    unordered_map<string, shared_value> const& simple_config_object::entry_set() const {
        return _value;
    }

    shared_object simple_config_object::with_value(path raw_path, shared_value value) const {
        string key = *raw_path.first();
        path next = raw_path.remainder();

        if (next.empty()) {
            return with_value(key, value);
        } else {
            if (_value.find(key) != _value.end()) {
                shared_value child = _value.at(key);
                if (dynamic_pointer_cast<const config_object>(child)) {
                    // if we have an object, add to it
                    return with_value(key, dynamic_pointer_cast<const config_object>(child))->with_value(next, value);
                }
            }
            // as soon as we have a non-object, replace it entirely
            shared_config subtree = value->at_path(
                    make_shared<simple_config_origin>("with_value(" + next.render() + ")"), next);
            return with_value(key, subtree->root());
        }
    }

    shared_object simple_config_object::without_path(path raw_path) const {
        string key = *raw_path.first();
        path next = raw_path.remainder();
        auto v = _value.find(key);

        auto object = v != _value.end() ? dynamic_pointer_cast<const config_object>((*v).second) : nullptr;
        if (object && !next.empty()) {
            auto value = object->without_path(next);
            unordered_map<string, shared_value> updated { make_pair(key, value) };
            return make_shared<simple_config_object>(origin(),
                                                     updated,
                                                     resolve_status_from_values(value_set(updated)),
                                                     _ignores_fallbacks);
        } else if (!next.empty() || v == _value.end()) {
            return dynamic_pointer_cast<const config_object>(shared_from_this());
        } else {
            unordered_map<string, shared_value> smaller;
            for (auto&& old : _value) {
                if (old.first != key) {
                    smaller.emplace(old);
                }
            }
            return make_shared<simple_config_object>(origin(),
                                                     smaller,
                                                     resolve_status_from_values(value_set(smaller)),
                                                     _ignores_fallbacks);
        }
    }

    shared_object simple_config_object::with_only_path(path raw_path) const {
        shared_object o = with_only_path_or_null(raw_path);
        if (!o) {
            return make_shared<simple_config_object>(origin(), unordered_map<string, shared_value> { },
                                                     resolve_status::RESOLVED, _ignores_fallbacks);
        } else {
            return o;
        }
    }

    shared_object simple_config_object::with_only_path_or_null(path raw_path) const {
        string key = *raw_path.first();
        path next = raw_path.remainder();
        auto v = _value.find(key);

        shared_object o;
        if (!next.empty()) {
            auto object = v != _value.end() ? dynamic_pointer_cast<const config_object>((*v).second) : nullptr;
            o = object->with_only_path_or_null(next);
        }

        if (o == nullptr) {
            return nullptr;
        } else {
            return make_shared<simple_config_object>(origin(),
                                                     unordered_map<string, shared_value> { make_pair(key, o) },
                                                     o->get_resolve_status(), _ignores_fallbacks);
        }
    }

    shared_object simple_config_object::with_value(std::string key, shared_value value) const {
        if (!value) {
            throw config_exception("Trying to store null config_value in a config_object");
        }

        unordered_map<string, shared_value> new_map;
        if (_value.empty()) {
            new_map.emplace(key, value);
        } else {
            new_map = _value;
            new_map.emplace(key, value);
        }

        return make_shared<simple_config_object>(origin(), new_map, _resolved, _ignores_fallbacks);
    }

    shared_value simple_config_object::new_copy(shared_origin origin) const {
        return make_shared<simple_config_object>(move(origin), _value, _resolved, _ignores_fallbacks);
    }

    bool simple_config_object::operator==(config_value const& other) const {
        return equals<simple_config_object>(other, [&](simple_config_object const& o) { return _value == o._value; });
    }

    resolve_result<shared_value>
    simple_config_object::resolve_substitutions(resolve_context const& context, resolve_source const& source) const
    {
        if (_resolved == resolve_status::RESOLVED) {
            return resolve_result<shared_value>(context, shared_from_this());
        }

        resolve_source source_with_parent = source.push_parent(dynamic_pointer_cast<const container>(shared_from_this()));

        resolve_modifier modifier{context, move(source_with_parent)};
        auto value = modify_may_throw(modifier);
        return resolve_result<shared_value>(modifier.context, value);
    }

    shared_ptr<simple_config_object> simple_config_object::modify(no_exceptions_modifier& modifier) const
    {
        return modify_may_throw(modifier);
    }

    shared_ptr<simple_config_object> simple_config_object::modify_may_throw(modifier& the_modifier) const
    {
        unordered_map<string, shared_value> changes;

        for (const auto& pair : _value) {
            auto& k = pair.first;
            auto& v = pair.second;
            auto modified = the_modifier.modify_child_may_throw(k, v);

            if (modified != v) {
                changes.emplace(k, modified);
            }
        }

        if (changes.empty()) {
            return const_pointer_cast<simple_config_object>(dynamic_pointer_cast<const simple_config_object>(shared_from_this()));
        } else {
            unordered_map<string, shared_value> modified;
            resolve_status status = resolve_status::RESOLVED;

            for (const auto& pair : _value) {
                auto& k = pair.first;
                auto& v = pair.second;

                auto iter = changes.find(k);
                if (iter != changes.end()) {
                    auto& new_value = iter->second;

                    if (new_value) {
                        modified.emplace(k, new_value);
                        if (new_value->get_resolve_status() == resolve_status::UNRESOLVED) {
                            status = resolve_status::UNRESOLVED;
                        }
                    }
                } else {
                    modified.emplace(k, v);
                    if (v->get_resolve_status() == resolve_status::UNRESOLVED) {
                        status = resolve_status::UNRESOLVED;
                    }
                }
            }
            return make_shared<simple_config_object>(origin(), move(modified), status, ignores_fallbacks());
        }
    }

    resolve_status simple_config_object::resolve_status_from_value(const unordered_map<string, shared_value>& value) {
        using pair = unordered_map<string, shared_value>::value_type;
        return any_of(value.begin(), value.end(), [](const pair& value) {
                 return value.second->get_resolve_status() == resolve_status::UNRESOLVED;
             }) ? resolve_status::UNRESOLVED : resolve_status::RESOLVED;
    }

    shared_value simple_config_object::replace_child(shared_value const &child, shared_value replacement) const {
        unordered_map<string, shared_value> new_children(_value);

        for (auto&& old : new_children) {
            if (old.second == child) {
                if (replacement) {
                    old.second = replacement;
                } else {
                    new_children.erase(old.first);
                }

                auto value_list = value_set(new_children);
                return make_shared<simple_config_object>(origin(),
                                                         move(new_children),
                                                         resolve_status_from_values(value_list),
                                                         ignores_fallbacks());
            }
        }
        throw bug_or_broken_exception("simple_config_object::replace_child did not find " + child->render());
    }

    bool simple_config_object::has_descendant(shared_value const &descendant) const {
        auto value_list = value_set(_value);
        for (auto&& child : value_list) {
            if (child == descendant) {
                return true;
            }
        }
        // now do the expensive search
        for (auto&& child : value_list) {
            if (auto c = dynamic_pointer_cast<const container>(child)) {
                if (c->has_descendant(descendant)) {
                    return true;
                }
            }
        }
        return false;
    }

    vector<string> simple_config_object::key_set() const {
        vector<string> keys;
        for (auto const& kv : _value) {
            keys.push_back(kv.first);
        }
        return keys;
    }

    vector<shared_value> simple_config_object::value_set(unordered_map<string, shared_value> m) const {
        vector<shared_value> values;
        for (auto const& kv : m) {
            values.push_back(kv.second);
        }
        return values;
    }

    shared_ptr<simple_config_object> simple_config_object::empty() {
        return empty_instance();
    }

    shared_ptr<simple_config_object> simple_config_object::empty(shared_origin origin) {
        if (origin == nullptr) {
            return empty();
        } else {
            return make_shared<simple_config_object>(move(origin), unordered_map<string, shared_value>());
        }
    }

    shared_ptr<simple_config_object> simple_config_object::empty_instance() {
        return empty(make_shared<simple_config_origin>("empty config"));
    }

    shared_value simple_config_object::with_fallbacks_ignored() const {
        if (_ignores_fallbacks) {
            return shared_from_this();
        } else {
            return make_shared<simple_config_object>(origin(), _value, _resolved, true);
        }
    }

    shared_value simple_config_object::merged_with_object(shared_object abstract_fallback) const {
        auto fallback = dynamic_pointer_cast<const simple_config_object>(abstract_fallback);
        if (!fallback) {
            throw bug_or_broken_exception("should not be reached (merging non-simple_config_object)");
        }

        bool changed = false;
        auto new_resolve_status = resolve_status::RESOLVED;
        auto merged = unordered_map<string, shared_value>();
        auto all_keys = [&]() {
            auto our_keys = key_set();
            auto fallback_keys = fallback->key_set();
            auto all_keys = unordered_set<string>(our_keys.begin(), our_keys.end());
            all_keys.insert(fallback_keys.begin(), fallback_keys.end());
            return all_keys;
        }();

        for (auto const& key : all_keys) {
            auto first = _value.find(key);
            auto second = fallback->_value.find(key);
            auto kept = [&]() {
                if (first == _value.end()) {
                    return second->second;
                } else if (second == fallback->_value.end()) {
                    return first->second;
                } else {
                    auto merge = dynamic_pointer_cast<const config_value>(first->second->with_fallback(second->second));
                    if (!merge) {
                        throw bug_or_broken_exception("Expected with_fallback to return same type of object");
                    }
                    return merge;
                }
            }();

            merged.insert(make_pair(key, kept));

            if (first == _value.end() || first->second != kept) {
                changed = true;
            }

            if (kept->get_resolve_status() == resolve_status::UNRESOLVED) {
                new_resolve_status = resolve_status::UNRESOLVED;
            }
        }

        bool new_ignores_fallbacks = fallback->ignores_fallbacks();

        if (changed) {
            return make_shared<simple_config_object>(merge_origins({shared_from_this(), fallback}),
                                                     merged, new_resolve_status, new_ignores_fallbacks);
        } else if (new_resolve_status != get_resolve_status() || new_ignores_fallbacks != ignores_fallbacks()) {
            return make_shared<simple_config_object>(origin(), _value, new_resolve_status, new_ignores_fallbacks);
        } else {
            return shared_from_this();
        }
    }

}  // namespace hocon
