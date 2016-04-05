#pragma once

#include <hocon/config_parse_options.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/nodes/config_node_root.hpp>
#include <internal/nodes/config_node_include.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/tokenizer.hpp>

#include <stack>

namespace hocon { namespace config_document_parser {

    std::shared_ptr<config_node_root> parse(token_iterator tokens,
                                            shared_origin origin,
                                            config_parse_options options);

    shared_node_value parse_value(token_iterator tokens,
                                  shared_origin origin,
                                  config_parse_options options);

    class parse_context {
    public:
        parse_context(config_syntax flavor, shared_origin origin, token_iterator tokens);

        std::shared_ptr<config_node_root> parse();

        /**
         * Parse a given input stream into a single value node. Used when doing a replace inside a ConfigDocument.
         */
        shared_node_value parse_single_value();

    private:
        parse_exception parse_error(std::string message);

        shared_token pop_token();
        shared_token next_token();
        shared_token next_token_collecting_whitespace(shared_node_list& nodes);
        void put_back(shared_token token);

        /**
         * In arrays and objects, comma can be omitted
         * as long as there's at least one newline instead.
         * this skips any newlines in front of a comma,
         * skips the comma, and returns true if it found
         * either a newline or a comma. The iterator
         * is left just after the comma or the newline.
         */
        bool check_element_separator(shared_node_list& nodes);

        /** Parse a concatenation. If there is no concatenation, return the next value. */
        shared_node_value consolidate_values(shared_node_list& nodes);

        std::string add_quote_suggestion(std::string bad_token, std::string message,
                                         bool inside_equals, path* last_path);

        std::string add_quote_suggestion(std::string bad_token, std::string message);

        shared_node_value parse_value(shared_token t);
        std::shared_ptr<config_node_path> parse_key(shared_token t);
        bool is_key_value_separator(shared_token t);
        std::shared_ptr<config_node_include> parse_include(shared_node_list& children);
        std::shared_ptr<config_node_complex_value> parse_object(bool had_open_curly);
        std::shared_ptr<config_node_complex_value> parse_array();

        static bool is_include_keyword(shared_token t);
        static bool is_unquoted_whitespace(shared_token t);
        static bool is_valid_array_element(shared_token t);

        int _line_number;
        std::stack<shared_token> _buffer;
        token_iterator _tokens;
        config_syntax _flavor;
        shared_origin _base_origin;

        // this is the number of "equals" we are inside,
        // used to modify the error message to reflect that
        // someone may think this is .properties format.
        int _equals_count;
    };


}}  // namespace hocon::config_document_parser
