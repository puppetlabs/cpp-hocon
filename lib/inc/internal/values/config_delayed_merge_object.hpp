#pragma once

#include <hocon/config_object.hpp>
#include <hocon/config_exception.hpp>

namespace hocon {

    class config_delayed_merge_object : public config_object {
    public:
        config_delayed_merge_object(shared_origin origin, std::vector<shared_value> const& stack);

        shared_object with_value(path raw_path, shared_value value) const override;
        shared_object with_value(std::string key, shared_value value) const override;

        resolve_status get_resolve_status() const override { return resolve_status::UNRESOLVED; }

        // map interface
        bool is_empty() const override { throw not_resolved(); }
        size_t size() const override { throw not_resolved(); }
        shared_value operator[](std::string const& key) const override { throw not_resolved(); }
        shared_value get(std::string const& key) const override { throw not_resolved(); }
        iterator begin() const override { throw not_resolved(); }
        iterator end() const override { throw not_resolved(); }

        bool operator==(config_value const& other) const override;

    protected:
        shared_value attempt_peek_with_partial_resolve(std::string const& key) const override;
        std::unordered_map<std::string, shared_value> const& entry_set() const override;
        shared_object without_path(path raw_path) const override;
        shared_object with_only_path(path raw_path) const override;
        shared_object with_only_path_or_null(path raw_path) const override;
        shared_value new_copy(shared_origin origin) const override;
        bool ignores_fallbacks() const override;

    private:
        not_resolved_exception not_resolved() const;

        const std::vector<shared_value> _stack;
    };

}  // namespace hocon::config_delayed_merge_object

