#pragma once

#include <hocon/config_object.hpp>
#include <hocon/config_exception.hpp>
#include <internal/replaceable_merge_stack.hpp>
#include <internal/values/config_delayed_merge.hpp>

namespace hocon {

    class config_delayed_merge_object : public config_object, public replaceable_merge_stack {
    public:
        config_delayed_merge_object(shared_origin origin, std::vector<shared_value> const& stack);

        shared_value make_replacement(resolve_context const& context, int skipping) const override;

        shared_object with_value(path raw_path, shared_value value) const override;
        shared_object with_value(std::string key, shared_value value) const override;

        resolve_status get_resolve_status() const override { return resolve_status::UNRESOLVED; }

        std::vector<std::string> key_set() const override { throw not_resolved(); }

        // map interface
        bool is_empty() const override { throw not_resolved(); }
        size_t size() const override { throw not_resolved(); }
        shared_value operator[](std::string const& key) const override { throw not_resolved(); }
        shared_value get(std::string const& key) const override { throw not_resolved(); }
        iterator begin() const override { throw not_resolved(); }
        iterator end() const override { throw not_resolved(); }
        unwrapped_value unwrapped() const override;

        bool operator==(config_value const& other) const override;

        // container interface
        shared_value replace_child(shared_value const& child, shared_value replacement) const override;
        bool has_descendant(shared_value const& descendant) const override;


    protected:
        shared_value attempt_peek_with_partial_resolve(std::string const& key) const override;
        std::unordered_map<std::string, shared_value> const& entry_set() const override;
        shared_object without_path(path raw_path) const override;
        shared_object with_only_path(path raw_path) const override;
        shared_object with_only_path_or_null(path raw_path) const override;
        shared_object new_copy(resolve_status const& status, shared_origin origin) const override;
        bool ignores_fallbacks() const override;
        virtual void render(std::string& result, int indent, bool at_root, std::string const& at_key, config_render_options options) const override;
        virtual void render(std::string& result, int indent, bool at_root, config_render_options options) const override;

    private:
        not_resolved_exception not_resolved() const;

        const std::vector<shared_value> _stack;
    };

}  // namespace hocon::config_delayed_merge_object

