#pragma once

#include "config_node_complex_value.hpp"

namespace hocon {

    class config_node_concatenation : public config_node_complex_value {
    public:
        config_node_concatenation(shared_node_list children);

        std::shared_ptr<const config_node_complex_value> new_node(shared_node_list nodes) const override;
    };

    using shared_node_concatenation = std::shared_ptr<const config_node_concatenation>;

}  // namespace hocon
