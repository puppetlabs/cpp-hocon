#pragma once

#include "abstract_config_node.hpp"

namespace hocon {

    enum class config_include_kind { URL, FILE, CLASSPATH, HEURISTIC };

    class config_node_include : public abstract_config_node {
    public:
        config_node_include(std::vector<std::unique_ptr<abstract_config_node>> children,
                            config_include_kind kind);

        std::vector<std::shared_ptr<token>> get_tokens() const override;

        std::vector<std::unique_ptr<abstract_config_node>> const& children() const;
        config_include_kind kind() const;
        std::string name() const;

    private:
        std::vector<std::unique_ptr<abstract_config_node>> _children;
        config_include_kind _kind;
    };

}  // namespace hocon
