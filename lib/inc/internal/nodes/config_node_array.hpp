#pragma once

#include "config_node_complex_value.hpp"

namespace hocon {

    class config_node_array : public config_node_complex_value {
    public:
        config_node_array(shared_node_list childlren);

        std::shared_ptr<config_node_complex_value> new_node(shared_node_list nodes) override;
    };

}  // namespace hocon
