#pragma once

#include "abstract_config_node.hpp"

namespace hocon {

    enum class config_include_kind { URL, FILE, CLASSPATH, HEURISTIC };

    /** Represents an include statement of the form "include include_kind(include_path)". */
    class config_node_include : public abstract_config_node {
    public:
        config_node_include(shared_node_list children,
                            config_include_kind kind);

        token_list get_tokens() const override;

        shared_node_list const& children() const;
        config_include_kind kind() const;
        std::string name() const;

    private:
        shared_node_list _children;
        config_include_kind _kind;
    };

}  // namespace hocon
