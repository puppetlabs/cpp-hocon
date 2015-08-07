#pragma once

#include <internal/token.hpp>
#include "abstract_config_node.hpp"

namespace hocon {

    class config_node_single_token : public abstract_config_node {
    public:
        config_node_single_token(std::shared_ptr<token> t);

        std::vector<std::shared_ptr<token>> get_tokens() const override;

        std::shared_ptr<token> get_token() const;

    private:
        std::shared_ptr<token> _token;
    };

}  // namespace hocon
