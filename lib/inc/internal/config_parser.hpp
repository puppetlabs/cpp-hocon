#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_syntax.hpp>
#include <hocon/config_include_context.hpp>
#include <hocon/config_parse_options.hpp>
#include <hocon/path.hpp>
#include <internal/nodes/config_node_concatenation.hpp>
#include <internal/nodes/config_node_root.hpp>
#include <internal/nodes/abstract_config_node_value.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/nodes/config_node_array.hpp>
#include <internal/nodes/config_node_include.hpp>
#include <internal/full_includer.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace hocon { namespace config_parser {

    shared_value parse(std::shared_ptr<const config_node_root> document,
            shared_origin origin,
            config_parse_options options,
            shared_include_context include_context);

    class parse_context {
        int _line_number;
        std::shared_ptr<const config_node_root> _document;
        std::shared_ptr<const full_includer> _includer;
        shared_include_context _include_context;
        config_syntax _flavor;
        shared_origin _base_origin, _line_origin;
        std::vector<path> _path_stack;

    public:
        parse_context(config_syntax flavor, shared_origin origin, std::shared_ptr<const config_node_root> document,
                std::shared_ptr<const full_includer> includer, shared_include_context include_context);

        shared_value parse();

        int array_count;

    private:
        static shared_object create_value_under_path(path p, shared_value value);
        shared_origin line_origin() const;
        path full_current_path() const;
        shared_value parse_value(shared_node_value n, std::vector<std::string>& comments);
        void parse_include(std::unordered_map<std::string, shared_value> &values, std::shared_ptr<const config_node_include> n);
        shared_object parse_object(shared_node_object n);
        shared_value parse_array(shared_node_array n);
        shared_value parse_concatenation(shared_node_concatenation n);
    };

}}  // namespace hocon::config_parser
