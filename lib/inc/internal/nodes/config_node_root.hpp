#pragma once

#include "config_node_complex_value.hpp"
#include <hocon/config_syntax.hpp>

namespace hocon {

    class config_node_root : public config_node_complex_value {
    public:
        config_node_root(shared_node_list children, shared_origin origin);

        std::shared_ptr<config_node_complex_value> new_node(shared_node_list nodes) override;

        std::shared_ptr<config_node_complex_value> value();
        std::shared_ptr<config_node_root> set_value(std::string desired_path, shared_node_value, config_syntax flavor);
        bool has_value(std::string desired_path);

    private:
        shared_origin _origin;
    };

}  // namespace hocon
