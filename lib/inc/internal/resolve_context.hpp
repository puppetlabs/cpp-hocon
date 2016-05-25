#pragma once

#include <hocon/types.hpp>
#include <hocon/config_resolve_options.hpp>
#include <hocon/path.hpp>

#include <unordered_map>

namespace hocon {

    class resolve_source;

    template<typename T>
    struct resolve_result;

    class resolve_context {
    public:
        resolve_context(config_resolve_options options, path restrict_to_child, std::vector<shared_value> cycle_markers);
        resolve_context(config_resolve_options options, path restrict_to_child);
        bool is_restricted_to_child() const;
        config_resolve_options options() const;

        resolve_result<shared_value> resolve(shared_value original, resolve_source const& source) const;
        path restrict_to_child() const;

        resolve_context add_cycle_marker(shared_value value) const;
        resolve_context remove_cycle_marker(shared_value value);
        resolve_context restrict(path restrict_to) const;
        resolve_context unrestricted() const;

        static shared_value resolve(shared_value value, shared_object root, config_resolve_options options);

    private:
        struct memo_key {
            shared_value value;
            path restrict_to_child;
            bool operator==(const memo_key& other) const {
                return value == other.value && restrict_to_child == other.restrict_to_child;
            }
        };

        struct memo_key_hash {
            std::size_t operator()(const memo_key&) const;
        };
        using resolve_memos = std::unordered_map<memo_key, shared_value, memo_key_hash>;
        config_resolve_options _options;
        path _restrict_to_child;
        resolve_memos _memos;
        std::vector<shared_value> _cycle_markers;

        resolve_context memoize(const memo_key& key, const shared_value& value) const;
    };
}  // namespace hocon
