#pragma once

#include "abstract_config_node_value.hpp"
#include <hocon/config_value.hpp>

namespace hocon {

    class config_node_simple_value : public abstract_config_node_value {
    public:
        config_node_simple_value(shared_token value);

        shared_token get_token() const;
        shared_value get_value() const;

        token_list get_tokens() const override;

    private:
        shared_token _token;
    };

}  // namespace hocon
