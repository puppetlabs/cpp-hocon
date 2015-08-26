#pragma once

#include <internal/nodes/abstract_config_node.hpp>
#include <hocon/path.hpp>
#include <internal/token.hpp>

namespace hocon {

    class config_node_path : public abstract_config_node {
    public:
        config_node_path(path node_path, token_list tokens);

        token_list get_tokens() const override;
        path get_path() const;

        config_node_path sub_path(int to_remove);
        config_node_path first();


    private:
        path _path;
        token_list _tokens;
    };

}  // namespace hocon
