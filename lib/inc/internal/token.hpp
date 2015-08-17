#pragma once

#include "simple_config_origin.hpp"

#include <string>

namespace hocon {

    enum class token_type {
        START, END, COMMA, EQUALS, COLON, OPEN_CURLY, CLOSE_CURLY, OPEN_SQUARE, CLOSE_SQUARE,
        VALUE, NEWLINE, UNQUOTED_TEXT, IGNORED_WHITESPACE, SUBSTITUTION, PROBLEM, COMMENT, PLUS_EQUALS
    };

    struct unsupported_exception : std::runtime_error {
        explicit unsupported_exception(std::string const& message);
    };

    class token {
    public:
        token(token_type type, shared_origin origin = nullptr,
              std::string token_text = "", std::string debug_string = "");

        virtual token_type get_token_type() const;
        virtual std::string token_text() const;
        virtual std::string to_string() const;
        virtual shared_origin const& origin() const;

        int line_number() const;

        virtual bool operator==(const token& other) const;

    private:
        token_type _token_type;

        /** For singleton tokens this is null. */
        shared_origin _origin;

        std::string _token_text;
        std::string _debug_string;
    };

    using shared_token = std::shared_ptr<const token>;
    using token_list = std::vector<shared_token>;

}  // namespace hocon
