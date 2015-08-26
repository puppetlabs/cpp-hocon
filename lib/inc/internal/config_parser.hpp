#pragma once

#include <hocon/config_syntax.hpp>
#include <hocon/config_include_context.hpp>
#include <hocon/config_parse_options.hpp>
#include <internal/path.hpp>
#include <internal/nodes/config_node_root.hpp>
#include <internal/nodes/abstract_config_node_value.hpp>
#include <internal/values/abstract_config_value.hpp>
#include <memory>
#include <stack>
#include <vector>
#include <string>

namespace hocon { namespace config_parser {
    shared_value parse(std::shared_ptr<const config_node_root> document,
            shared_origin origin,
            shared_parse_options const& options,
            std::shared_ptr<config_include_context> include_context);

    class parse_context {
        int _line_number;
        std::shared_ptr<const config_node_root> _document;
        // void* _includer;
        std::shared_ptr<config_include_context> _include_context;
        // config_syntax _flavor;
        shared_origin _base_origin;
        std::stack<path> _path_stack;

    public:
        parse_context(config_syntax flavor, shared_origin origin, std::shared_ptr<const config_node_root> document,
                void* includer, std::shared_ptr<config_include_context> include_context);

        shared_value parse();

        int array_count;

    private:
        shared_value parse_value(shared_node_value n, std::vector<std::string> comments);
    };

}}  // namespace hocon::config_parser
