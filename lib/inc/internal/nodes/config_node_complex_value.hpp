#pragma once

#include "abstract_config_node_value.hpp"

namespace hocon {

    class config_node_complex_value : public abstract_config_node_value {
    public:
        config_node_complex_value(std::vector<std::shared_ptr<abstract_config_node>> children);

        std::vector<std::shared_ptr<token>> get_tokens() const override;

        std::vector<std::shared_ptr<abstract_config_node>> const& children() const;

        std::shared_ptr<config_node_complex_value> indent_text(
                std::shared_ptr<abstract_config_node> indentation);

        virtual std::shared_ptr<config_node_complex_value> new_node(
                std::vector<std::shared_ptr<abstract_config_node>> nodes) = 0;

    private:
        std::vector<std::shared_ptr<abstract_config_node>> _children;
    };

}  // namespace hocon
