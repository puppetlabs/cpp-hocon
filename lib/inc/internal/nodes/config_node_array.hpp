#pragma once

#include "config_node_complex_value.hpp"

namespace hocon {

    class config_node_array;
    using shared_node_array = std::shared_ptr<const config_node_array>;

    class config_node_array : public config_node_complex_value {
    public:
        config_node_array(shared_node_list children);

        std::shared_ptr<const config_node_complex_value> new_node(shared_node_list nodes) const override;
    };

}  // namespace hocon
