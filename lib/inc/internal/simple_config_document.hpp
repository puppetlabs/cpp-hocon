#pragma once

#include <hocon/config_document.hpp>
#include <memory>

namespace hocon {
    class config_node_root;
    class config_parse_options;

    class simple_config_document : public config_document {
    public:
        simple_config_document(std::unique_ptr<config_node_root> root, config_parse_options opts);

        config_document with_value_text(std::string path, std::string new_value) const override;
        config_document with_value(std::string path, config_value new_value) const override;
        config_document without_path(std::string path) const override;

        bool has_path(std::string const& path) const override;

        std::string render() const override;

    private:
        std::unique_ptr<config_node_root> _config_node_tree;
        config_parse_options _parse_options;
    };
}
