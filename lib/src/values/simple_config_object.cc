#include <internal/values/simple_config_object.hpp>
#include <hocon/config_value.hpp>
#include <hocon/config_exception.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_source.hpp>
#include <internal/resolve_result.hpp>
#include <internal/container.hpp>

#include <algorithm>

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
        return _value.at(key);
    }

    bool simple_config_object::is_empty() const {
        return _value.empty();
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
            // TODO: the last argument is incorrect, fix when implementing resolve functionality
            return make_shared<simple_config_object>(origin(), updated, resolve_status::RESOLVED, _ignores_fallbacks);
        } else if (!next.empty() || v == _value.end()) {
            return dynamic_pointer_cast<const config_object>(shared_from_this());
        } else {
            unordered_map<string, shared_value> smaller;
            for (auto&& old : _value) {
                if (old.first != key) {
                    smaller.emplace(old);
                }
            }
            // TODO: the last argument is incorrect, fix when implementing resolve functionality
            return make_shared<simple_config_object>(origin(), smaller);
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
        // TODO: implement
        throw config_exception("simple_config_object::replace_child not implemented");
    }

    bool simple_config_object::has_descendant(shared_value const &descendant) const {
        // TODO: implement
        throw config_exception("simple_config_object::has_descendant not implemented");
    }



}  // namespace hocon
