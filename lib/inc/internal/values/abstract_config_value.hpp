#pragma once

#include <hocon/config_value.hpp>
#include <internal/simple_config_origin.hpp>

namespace hocon {

    enum class resolved_status { RESOLVED, UNRESOLVED };

    class abstract_config_value : public config_value {
    public:
        abstract_config_value(shared_origin origin);

        virtual std::string transform_to_string() const;
        virtual resolved_status resolved() const;

        std::string render() const override;
        std::string render(config_render_options options) const override;
        void render(std::string& result, int indent, bool at_root, std::string at_key,
                    config_render_options options) const;
        virtual void render(std::string& result, int indent, bool at_root, config_render_options options) const;

        shared_origin const& origin() const;

    private:
        shared_origin _origin;
    };

    using shared_value = std::shared_ptr<const abstract_config_value>;

}  // namespace hocon
