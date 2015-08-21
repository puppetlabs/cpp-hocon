#pragma once

#include <hocon/parser/config_document.hpp>
#include <memory>

namespace hocon {
    class config_node_root;
    class config_parse_options;

    class simple_config_document : public config_document {
    public:
        simple_config_document(std::shared_ptr<config_node_root> root, std::shared_ptr<config_parse_options> opts);

        std::unique_ptr<config_document> with_value_text(std::string path, std::string new_value) const override;
        std::unique_ptr<config_document> with_value(std::string path, config_value new_value) const override;
        std::unique_ptr<config_document> without_path(std::string path) const override;

        bool has_path(std::string const& path) const override;

        std::string render() const override;

    private:
        std::shared_ptr<config_node_root> _config_node_tree;
        std::shared_ptr<config_parse_options> _parse_options;
    };
}
