#pragma once

#include "token.hpp"
#include <internal/values/abstract_config_value.hpp>

namespace hocon {

    class value : public token {
    public:
        value(std::unique_ptr<abstract_config_value> value);
        value(std::unique_ptr<abstract_config_value> value, std::string original_text);

        std::string to_string() const override;
        std::shared_ptr<simple_config_origin> const& origin() const override;

        std::unique_ptr<abstract_config_value> const& get_value() const;

        bool operator==(const token& other) const override;

    private:
        std::unique_ptr<abstract_config_value> _value;
    };

    class line : public token {
    public:
        line(std::shared_ptr<simple_config_origin> origin);

        std::string to_string() const override;

        bool operator==(token const& other) const override;
    };

    class unquoted_text : public token {
    public:
        unquoted_text(std::shared_ptr<simple_config_origin> origin, std::string text);

        std::string to_string() const override;

        bool operator==(const token& other) const override;
    };

    class ignored_whitespace : public token {
    public:
        ignored_whitespace(std::shared_ptr<simple_config_origin> origin, std::string whitespace);

        std::string to_string() const override;

        bool operator==(const token& other) const override;
    };

    class problem : public token {
    public:
        problem(std::shared_ptr<simple_config_origin> origin, std::string what, std::string message,
            bool suggest_quotes);

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
        comment(std::shared_ptr<simple_config_origin> origin, std::string text);

        std::string text() const;

        std::string to_string() const override;
        bool operator==(const token& other) const override;

    private:
        std::string _text;
    };

    class double_slash_comment : public comment {
    public:
        double_slash_comment(std::shared_ptr<simple_config_origin> origin, std::string text);

        std::string token_text() const override;
    };

    class hash_comment : public comment {
    public:
        hash_comment(std::shared_ptr<simple_config_origin> origin, std::string text);

        std::string token_text() const override;
    };

    class substitution : public token {
    public:
        substitution(std::shared_ptr<simple_config_origin> origin,
                     bool optional, std::vector<std::shared_ptr<token>> expression);

        bool optional() const;
        std::vector<std::shared_ptr<token>> const& expression() const;

        std::string token_text() const override;
        std::string to_string() const override;

        bool operator==(const token& other) const override;

    private:
        bool _optional;
        std::vector<std::shared_ptr<token>> _expression;
    };

    /** Free functions */
    bool isValueWithType(token t, config_value_type type);

    class tokens {
    public:
        /** Singleton tokens */
        static std::shared_ptr<token> const& start_token();
        static std::shared_ptr<token> const& end_token();
        static std::shared_ptr<token> const& comma_token();
        static std::shared_ptr<token> const& equals_token();
        static std::shared_ptr<token> const& colon_token();
        static std::shared_ptr<token> const& open_curly_token();
        static std::shared_ptr<token> const& close_curly_token();
        static std::shared_ptr<token> const& open_square_token();
        static std::shared_ptr<token> const& close_square_token();
        static std::shared_ptr<token> const& plus_equals_token();
    };

}  // namespace hocon
