#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_list.hpp>
#include <hocon/config_render_options.hpp>
#include <internal/container.hpp>
#include <algorithm>
#include <memory>
#include <vector>
#include <boost/optional.hpp>

namespace hocon {

    class simple_config_list : public config_list, public container {
    public:
        // This would be useful if we need to iterator over all the values in a list
        // We don't know if we need it yet
        using iterator = std::vector<shared_value>::const_iterator;

        simple_config_list(shared_origin origin, std::vector<shared_value> value);
        simple_config_list(shared_origin origin, std::vector<shared_value> value, resolve_status status);

        config_value_type value_type() const override { return config_value_type::LIST; }
        // unwrapped()
        resolve_status get_resolve_status() const override { return _resolved; }

        shared_value replace_child(shared_value const& child, shared_value replacement) const override;
        bool has_descendant(shared_value const& descendant) const override;

        shared_value relativized(const std::string prefix) const override;

        bool contains(shared_value v) const { return std::find(_value.begin(), _value.end(), v) != _value.end(); }
        bool contains_all(std::vector<shared_value>) const;
        shared_value get(int index) const { return _value[index]; }

        int index_of(shared_value v) {
            auto pos = find(_value.begin(), _value.end(), v);
            if (pos == _value.end()) {
                return -1;
            } else {
                return pos - _value.begin();
            }
        }

        bool is_empty() const { return _value.empty(); }
        size_t size() const { return _value.size(); }
        iterator begin() const { return _value.begin(); }
        iterator end() const { return _value.end(); }

        std::shared_ptr<const simple_config_list> concatenate(std::shared_ptr<const simple_config_list> other) const;


    protected:
        resolve_result<shared_value>
            resolve_substitutions(resolve_context const& context, resolve_source const& source) const override;
        shared_value new_copy(shared_origin origin) const override;

        void render(std::string& result, int indent, bool at_root, config_render_options options) const override;
        bool operator==(config_value const& other) const override;

    private:
        static const long _serial_version_UID = 2L;
        const std::vector<shared_value> _value;
        const resolve_status _resolved;

        std::shared_ptr<const simple_config_list>
        modify(no_exceptions_modifier& modifier, boost::optional<resolve_status> new_resolve_status) const;

        std::shared_ptr<const simple_config_list>
        modify_may_throw(modifier& modifier, boost::optional<resolve_status> new_resolve_status) const;

        struct resolve_modifier;
    };

}   // namespace hocon
