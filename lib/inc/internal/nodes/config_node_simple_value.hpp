#pragma once

#include "abstract_config_node.hpp"
#include <internal/values/abstract_config_value.hpp>

namespace hocon {

    class config_node_simple_value : public abstract_config_node {
    public:
        config_node_simple_value(std::shared_ptr<token> value);

        std::shared_ptr<token> get_token() const;
        std::unique_ptr<abstract_config_value> const& get_value() const;

        std::vector<std::shared_ptr<token>> get_tokens() const override;

    private:
        std::shared_ptr<token> _value;
    };

}  // namespace hocon