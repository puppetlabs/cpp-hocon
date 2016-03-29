#pragma once

#include "token.hpp"
#include <hocon/config_value.hpp>

namespace hocon {

    class value : public token {
    public:
        value(shared_value value);
        value(shared_value value, std::string original_text);

        std::string to_string() const override;
        shared_origin const& origin() const override;

        shared_value get_value() const;

        bool operator==(const token& other) const override;

    private:
        shared_value _value;
    };

    class line : public token {
    public:
        line(shared_origin origin);

        std::string to_string() const override;

        bool operator==(token const& other) const override;
    };

    class unquoted_text : public token {
    public:
        unquoted_text(shared_origin origin, std::string text);

        std::string to_string() const override;

        bool operator==(const token& other) const override;
    };

    class ignored_whitespace : public token {
    public:
        ignored_whitespace(shared_origin origin, std::string whitespace);

        std::string to_string() const override;

        bool operator==(const token& other) const override;
    };

    class problem : public token {
    public:
        problem(shared_origin origin, std::string what, std::string message, bool suggest_quotes);

        std::string what() const;
        std::string message() const;
        bool suggest_quotes() const;

        std::string to_string() const override;

        bool operator==(const token& other) const override;

    private:
        std::string _what;
        std::string _message;
        bool _suggest_quotes;
    };

    class comment : public token {
    public:
        comment(shared_origin origin, std::string text);

        std::string text() const;

        std::string to_string() const override;
        bool operator==(const token& other) const override;

    private:
        std::string _text;
    };

    class double_slash_comment : public comment {
    public:
        double_slash_comment(shared_origin origin, std::string text);

        std::string token_text() const override;
    };

    class hash_comment : public comment {
    public:
        hash_comment(shared_origin origin, std::string text);

        std::string token_text() const override;
    };

    class substitution : public token {
    public:
        substitution(shared_origin origin, bool optional, token_list expression);

        bool optional() const;
        token_list const& expression() const;

        std::string token_text() const override;
        std::string to_string() const override;

        bool operator==(const token& other) const override;

    private:
        bool _optional;
        token_list _expression;
    };

    class tokens {
    public:
        /** Singleton tokens */
        static shared_token const& start_token();
        static shared_token const& end_token();
        static shared_token const& comma_token();
        static shared_token const& equals_token();
        static shared_token const& colon_token();
        static shared_token const& open_curly_token();
        static shared_token const& close_curly_token();
        static shared_token const& open_square_token();
        static shared_token const& close_square_token();
        static shared_token const& plus_equals_token();

        static bool is_newline(shared_token);
        static bool is_ignored_whitespace(shared_token);

        static bool is_value_with_type(shared_token t, config_value::type type);

        static shared_value get_value(shared_token t);
    };

}  // namespace hocon
