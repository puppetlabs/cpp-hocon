#include <internal/config_delayed_merge_object.hpp>
#include <internal/config_delayed_merge.hpp>
#include <internal/config_exception.hpp>

using namespace std;

namespace hocon {

    config_delayed_merge_object::config_delayed_merge_object(shared_origin origin, vector<shared_value> const &stack) :
        config_object(move(origin)), _stack(move(stack)) {
        if (_stack.empty()) {
            throw config_exception("creating empty delayed merge object");
        }

        if (!dynamic_pointer_cast<const config_object>(_stack.front())) {
            throw config_exception("created a delayed merge object not guaranteed to be an object");
        }

        for (auto& v : _stack) {
            if (dynamic_pointer_cast<const config_delayed_merge>(v) || dynamic_pointer_cast<const config_delayed_merge_object>(v)) {
                throw config_exception("placed nested delayed_merge in a config_delayed_merge_object, should have consolidated stack");
            }
        }
    }

    shared_object config_delayed_merge_object::with_value(path raw_path, shared_value value) const {
        // TODO
        return {};
    }

    shared_object config_delayed_merge_object::with_value(string key, shared_value value) const {
        // TODO
        return {};
    }

    shared_value config_delayed_merge_object::new_copy(shared_origin origin) const {
        // TODO
        throw config_exception("config_delayed_merge_object::new_copy not implemented");
    }

    shared_value config_delayed_merge_object::attempt_peek_with_partial_resolve(string const& key) const {
        // TODO
        return {};
    }

    bool config_delayed_merge_object::is_empty() const {
        // TODO
        return {};
    }

    unordered_map<string, shared_value> const& config_delayed_merge_object::entry_set() const {
        // TODO
        throw config_exception("");
    }

    shared_object config_delayed_merge_object::without_path(path raw_path) const {
        // TODO
        return {};
    }

    shared_object config_delayed_merge_object::with_only_path(path raw_path) const {
        // TODO
        return {};
    }

    shared_object config_delayed_merge_object::with_only_path_or_null(path raw_path) const {
        // TODO
        return {};
    }

}  // namespace hocon
