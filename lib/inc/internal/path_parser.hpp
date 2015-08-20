#pragma once

#include "path_builder.hpp"
#include "simple_config_origin.hpp"
#include "tokenizer.hpp"
#include <hocon/config_syntax.hpp>
#include <internal/nodes/config_node_path.hpp>

#include <string>
#include <vector>

namespace hocon  {

    class path_parser {
    public:
        static config_node_path parse_path_node(std::string const& path_string,
                                                config_syntax flavor = config_syntax::CONF);

        static path parse_path(std::string const& path_string);

        static path parse_path_expression(iterator& expression, shared_origin origin,
                                          std::string const& original_text = "",
                                          token_list* path_tokens = nullptr,
                                          config_syntax flavor = config_syntax::CONF);

        static config_node_path parse_path_node_expression(iterator& expression,
                                                           shared_origin origin,
                                                           std::string const& original_text = "",
                                                           config_syntax flavor = config_syntax::CONF);

    private:
        class element {
        public:
            element(std::string initial, bool can_be_empty);

            std::string to_string() const;

            std::string _value;
            bool _can_be_empty;
        };

        static token_list split_token_on_period(shared_token t, config_syntax flavor);

        static void add_path_text(std::vector<element>& buff, bool was_quoted, std::string new_text);

        static bool looks_unsafe_for_fast_parser(std::string s);

        static path fast_path_build(path tail, std::string s);

        static path speculative_fast_parse_path(std::string const& path);

        static const shared_origin api_origin;
    };

}  // namespace hocon
