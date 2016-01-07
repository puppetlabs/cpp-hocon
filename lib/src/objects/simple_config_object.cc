#include <internal/objects/simple_config_object.hpp>
#include <hocon/config_value.hpp>
#include <internal/exception.hpp>
#include <internal/simple_config_origin.hpp>

using namespace std;

namespace hocon {

    simple_config_object::simple_config_object(shared_origin origin,
                                               unordered_map <std::string, shared_value> value,
                                               resolve_status status, bool ignores_fallbacks) :
        config_object(move(origin)), _value(move(value)), _ignores_fallbacks(ignores_fallbacks) { }

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
            // TODO: the last arugment is incorrect, fix when implementing resolve functionality
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
            // TODO: the last arugment is incorrect, fix when implementing resolve functionality
            return make_shared<simple_config_object>(origin(), smaller, resolve_status::RESOLVED);
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

        // TODO: the resolved arugment is incorrect, fix when implementing resolve functionality
        return make_shared<simple_config_object>(origin(), new_map, resolve_status::RESOLVED, _ignores_fallbacks);
    }

}  // namespace hocon
