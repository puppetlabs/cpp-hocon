#pragma once

#include "abstract_config_node_value.hpp"
#include "config_node_path.hpp"


namespace hocon {

    /**
     * A field represents a key-value pair of the format "key : value",
     * where key is a quoted or unquoted string, and value can be any node type.
     */
    class config_node_field : public abstract_config_node_value {
    public:
        config_node_field(shared_node_list children);

        token_list get_tokens() const override;

        std::shared_ptr<const config_node_field> replace_value(shared_node_value new_value) const;
        shared_node_value get_value() const;
        shared_token separator() const;
        std::vector<std::string> comments() const;
        std::shared_ptr<const config_node_path> path() const;

    private:
        shared_node_list _children;
    };

}  // namespace hocon
