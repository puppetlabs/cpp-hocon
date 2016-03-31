#pragma once

#include "tokens.hpp"
#include "hocon/config_exception.hpp"
#include <hocon/config_syntax.hpp>

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

    class iterator {
    public:
        virtual bool has_next() = 0;
        virtual shared_token next() = 0;
    };

    template <typename iter>
    class iterator_wrapper : public iterator {
    public:
        iterator_wrapper(iter begin, iter end)
            : _cur(begin), _end(end) { }

        bool has_next() override {
            return _cur != _end;
        }

        shared_token next() override {
            return *_cur++;
        }

    private:
        iter _cur;
        iter _end;
    };

    class token_iterator : public iterator {
    public:
        token_iterator(shared_origin origin, std::unique_ptr<std::istream> input, bool allow_comments);
        token_iterator(shared_origin origin, std::unique_ptr<std::istream> input, config_syntax flavor);

        bool has_next() override;
        shared_token next() override;

        static std::string render(token_list tokens);

    private:
        class whitespace_saver {
        public:
            whitespace_saver();
            void add(char c);
            shared_token check(token_type type, shared_origin base_origin, int line_number);

        private:
            shared_token next_is_not_simple_value(shared_origin base_origin, int line_number);
            shared_token next_is_simple_value(shared_origin origin, int line_number);
            shared_token create_whitespace_token(shared_origin base_origin, int line_number);

            std::string _whitespace;
            bool _last_token_was_simple_value;
        };

        char next_char_raw();
        void put_back(char c);
        bool start_of_comment(char c);
        shared_token pull_comment(char first_char);

        /** Get next char, skipping newline whitespace */
        char next_char_after_whitespace(whitespace_saver& saver);

        /**
         * The rules here are intended to maximize convenience while
         * avoiding confusion with real valid JSON. Basically anything
         * that parses as JSON is treated the JSON way and otherwise
         * we assume it's a string and let the parser sort it out.
         */
        shared_token pull_unquoted_text();

        shared_token pull_number(char first_char);

        /**
         * @param parsed The string with the escape sequence parsed.
         * @param original The string with the escape sequence left as in the original text
         */
        void pull_escape_sequence(std::string& parsed, std::string& original);

        void append_triple_quoted_string(std::string& parsed, std::string& original);

        shared_token pull_quoted_string();

        shared_token const& pull_plus_equals();
        shared_token pull_substitution();
        shared_token pull_next_token(whitespace_saver& saver);
        void queue_next_token();

        static bool is_simple_value(token_type type);
        static std::string as_string(char c);
        static shared_origin line_origin(shared_origin base_origin, int line_number);

        shared_origin _origin;
        std::unique_ptr<std::istream> _input;
        std::vector<char> _buffer;
        bool _allow_comments;
        int _line_number;
        shared_origin _line_origin;
        std::queue<shared_token> _tokens;
        whitespace_saver _whitespace_saver;
    };

    class single_token_iterator : public iterator {
    public:
        single_token_iterator(shared_token token);

        bool has_next() override;
        shared_token next() override;

    private:
        shared_token _token;
        bool _has_next;
    };

    class token_list_iterator : public iterator {
    public:
        token_list_iterator(token_list tokens);

        bool has_next() override;
        shared_token next() override;

    private:
        token_list _tokens;
        int _index;
    };

}  // namespace hocon
