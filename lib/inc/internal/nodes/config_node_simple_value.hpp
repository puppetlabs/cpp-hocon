#pragma once

#include "abstract_config_node.hpp"
#include <internal/values/abstract_config_value.hpp>

namespace hocon {

    class config_node_simple_value : public abstract_config_node {
    public:
        config_node_simple_value(shared_token value);

        shared_token get_token() const;
        std::unique_ptr<abstract_config_value> const& get_value() const;

        token_list get_tokens() const override;

    private:
        shared_token _value;
    };

}  // namespace hocon
