#pragma once

#include "tokens.hpp"
#include "config_exception.hpp"

#include <boost/nowide/fstream.hpp>
#include <vector>
#include <queue>
#include <string>

namespace hocon {

    // This exception should not leave this file
    class problem_exception : std::runtime_error {
    public:
        problem_exception(problem prob);
        problem const& get_problem() const;

    private:
        problem _problem;
    };

    class token_iterator {
    public:
        token_iterator(simple_config_origin origin, std::unique_ptr<std::istream> input, bool allow_comments);

        bool has_next();
        std::shared_ptr<token> next();

    private:
        class whitespace_saver {
        public:
            whitespace_saver();
            void add(char c);
            std::shared_ptr<token> check(token_type type, simple_config_origin const& base_origin,
                                         int line_number);

        private:
            std::shared_ptr<token> next_is_not_simple_value(simple_config_origin const& base_origin,
                                                            int line_number);
            std::shared_ptr<token> next_is_simple_value(simple_config_origin const& origin,
                                                        int line_number);
            std::shared_ptr<token> create_whitespace_token(simple_config_origin const& base_origin,
                                                           int line_number);

            std::string _whitespace;
            bool _last_token_was_simple_value;
        };

        char next_char_raw();
        void put_back(char c);
        bool start_of_comment(char c);
        std::shared_ptr<token> pull_comment(char first_char);

        /** Get next char, skipping newline whitespace */
        char next_char_after_whitespace(whitespace_saver& saver);

        /**
         * The rules here are intended to maximize convenience while
         * avoiding confusion with real valid JSON. Basically anything
         * that parses as JSON is treated the JSON way and otherwise
         * we assume it's a string and let the parser sort it out.
         */
        std::shared_ptr<token> pull_unquoted_text();

        std::shared_ptr<token> pull_number(char first_char);

        /**
         * @param parsed The string with the escape sequence parsed.
         * @param original The string with the escape sequence left as in the original text
         */
        void pull_escape_sequence(std::string& parsed, std::string& original);

        void append_triple_quoted_string(std::string& parsed, std::string& original);

        std::shared_ptr<token> pull_quoted_string();

        std::shared_ptr<token> const& pull_plus_equals();
        std::shared_ptr<token> pull_substitution();
        std::shared_ptr<token> pull_next_token(whitespace_saver& saver);
        void queue_next_token();

        static bool is_simple_value(token_type type);
        static std::string as_string(char c);
        static std::shared_ptr<simple_config_origin> line_origin(simple_config_origin const& base_origin,
                                                                 int line_number);

        simple_config_origin _origin;
        std::unique_ptr<std::istream> _input;
        std::vector<char> _buffer;
        bool _allow_comments;
        int _line_number;
        std::shared_ptr<simple_config_origin> _line_origin;
        std::queue<std::shared_ptr<token>> _tokens;
        whitespace_saver _whitespace_saver;
    };

}  // namespace hocon
