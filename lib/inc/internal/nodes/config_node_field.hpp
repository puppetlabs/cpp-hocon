#pragma once

#include "abstract_config_node_value.hpp"


namespace hocon {

    class config_node_field : public abstract_config_node_value {
    public:
        config_node_field(std::vector<std::shared_ptr<abstract_config_node>> children);

        std::vector<std::shared_ptr<token>> get_tokens() const override;

        std::shared_ptr<config_node_field> replace_value(std::shared_ptr<abstract_config_node_value> new_value);
        std::shared_ptr<abstract_config_node_value> get_value() const;
        std::shared_ptr<token> separator() const;
        std::vector<std::string> comments() const;

        //  TODO: implement path() once we have config_node_path


    private:
        std::vector<std::shared_ptr<abstract_config_node>> _children;
    };

}  // namespace hocon
