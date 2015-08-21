#pragma once

#include "config_node_complex_value.hpp"
#include "config_node_path.hpp"
#include <internal/path.hpp>
#include <hocon/config_syntax.hpp>

namespace hocon {

    class config_node_object : public config_node_complex_value {
    public:
        config_node_object(shared_node_list children);

        std::shared_ptr<config_node_complex_value> new_node(
                shared_node_list nodes) override;

        bool has_value(path desired_path) const;

        std::shared_ptr<config_node_object> change_value_on_path(path desired_path,
                                                                 shared_node_value value,
                                                                 config_syntax flavor);

        std::shared_ptr<config_node_object> set_value_on_path(std::string desired_path,
                                                              shared_node_value value,
                                                              config_syntax flavor = config_syntax::CONF);

        std::shared_ptr<config_node_object> set_value_on_path(config_node_path desired_path, shared_node_value value,
                                                              config_syntax flavor = config_syntax::CONF);

        shared_node_list indentation();

        std::shared_ptr<config_node_object> add_value_on_path(config_node_path desired_path,
                                                              shared_node_value value,
                                                              config_syntax flavor);

        std::shared_ptr<config_node_object> remove_value_on_path(std::string desired_path, config_syntax flavor);

        static bool contains_token(shared_node node, token_type token);
    };

}  // namespace hocon
