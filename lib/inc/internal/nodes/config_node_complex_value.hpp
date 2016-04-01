#pragma once

#include "abstract_config_node_value.hpp"

namespace hocon {

    class config_node_complex_value : public abstract_config_node_value {
    public:
        config_node_complex_value(shared_node_list children);

        token_list get_tokens() const override;

        shared_node_list const& children() const;

        std::shared_ptr<const config_node_complex_value> indent_text(
                shared_node indentation) const;

        virtual std::shared_ptr<const config_node_complex_value> new_node(
                shared_node_list nodes) const = 0;

    private:
        shared_node_list _children;
    };

}  // namespace hocon
