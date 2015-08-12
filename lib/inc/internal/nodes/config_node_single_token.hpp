#pragma once

#include <internal/token.hpp>
#include "abstract_config_node.hpp"

namespace hocon {

    class config_node_single_token : public abstract_config_node {
    public:
        config_node_single_token(shared_token t);

        token_list get_tokens() const override;

        shared_token get_token() const;

    private:
        shared_token _token;
    };

}  // namespace hocon
