#pragma once

#include <hocon/config_object.hpp>

namespace hocon {

    class config_delayed_merge_object : public config_object {
    public:
        config_delayed_merge_object(shared_origin origin, std::vector<shared_value> const& stack);

        shared_object with_value(path raw_path, shared_value value) const override;
        shared_object with_value(std::string key, shared_value value) const override;

    protected:
        shared_value attempt_peek_with_partial_resolve(std::string const& key) const override;
        bool is_empty() const override;
        std::unordered_map<std::string, shared_value> const& entry_set() const override;
        shared_object without_path(path raw_path) const override;
        shared_object with_only_path(path raw_path) const override;
        shared_object with_only_path_or_null(path raw_path) const override;

    private:
        const std::vector<shared_value> _stack;
    };

}  // namespace hocon::config_delayed_merge_object

