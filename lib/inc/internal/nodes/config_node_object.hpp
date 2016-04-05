#pragma once

#include "config_node_complex_value.hpp"
#include "config_node_path.hpp"
#include <hocon/path.hpp>
#include <hocon/config_syntax.hpp>

namespace hocon {

    class config_node_object;
    using shared_node_object = std::shared_ptr<const config_node_object>;

    class config_node_object : public config_node_complex_value {
    public:
        config_node_object(shared_node_list children);

        std::shared_ptr<const config_node_complex_value> new_node(
                shared_node_list nodes) const override;

        bool has_value(path desired_path) const;

        shared_node_object change_value_on_path(path desired_path,
                                                shared_node_value value,
                                                config_syntax flavor) const;

        shared_node_object set_value_on_path(std::string desired_path,
                                             shared_node_value value,
                                             config_syntax flavor = config_syntax::CONF) const;

        shared_node_object set_value_on_path(config_node_path desired_path,
                                             shared_node_value value,
                                             config_syntax flavor = config_syntax::CONF) const;

        shared_node_list indentation() const;

        shared_node_object add_value_on_path(config_node_path desired_path,
                                             shared_node_value value,
                                             config_syntax flavor) const;

        shared_node_object remove_value_on_path(std::string desired_path, config_syntax flavor) const;

        static bool contains_token(shared_node node, token_type token);
    };

}  // namespace hocon
