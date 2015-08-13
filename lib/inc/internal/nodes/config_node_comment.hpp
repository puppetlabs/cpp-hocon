#pragma once

#include "config_node_single_token.hpp"

namespace hocon {

    class config_node_comment : public config_node_single_token {
    public:
        config_node_comment(shared_token comment);

        std::string comment_text() const;
    };

}  // namespace hocon
