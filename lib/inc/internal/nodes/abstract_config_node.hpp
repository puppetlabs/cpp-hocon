#pragma once

#include <hocon/parser/config_node.hpp>
#include <internal/token.hpp>
#include <vector>

namespace hocon {

    class abstract_config_node : public config_node {
    public:
        std::string render() const;
        bool operator==(const abstract_config_node &other) const;
        virtual token_list get_tokens() const = 0;
    };

    using shared_node = std::shared_ptr<abstract_config_node>;
    using shared_node_list = std::vector<shared_node>;

}  // namespace hocon
