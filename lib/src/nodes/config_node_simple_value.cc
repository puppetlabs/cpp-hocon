#include <internal/nodes/config_node_simple_value.hpp>
#include <internal/config_exception.hpp>
#include <internal/tokens.hpp>
#include <internal/values/config_string.hpp>

using namespace std;

namespace hocon {

    config_node_simple_value::config_node_simple_value(shared_token value) : _token(move(value)) { }

    shared_token config_node_simple_value::get_token() const {
        return _token;
    }

    token_list config_node_simple_value::get_tokens() const {
        return token_list { _token };
    }

    shared_value config_node_simple_value::get_value() const {
        if (auto value_token = dynamic_pointer_cast<const value>(_token)) {
            return value_token->get_value();
        }

        if (auto text_token = dynamic_pointer_cast<const unquoted_text>(_token)) {
            return make_shared<config_string>(
                    text_token->origin(), text_token->token_text(), config_string_type::UNQUOTED);
        }

        if (auto sub_token = dynamic_pointer_cast<const substitution>(_token)) {
            token_list expression = sub_token->expression();
               // TODO: this will require Path and ConfigReference to be ported to handle properly
        }

        throw config_exception("Tried to get a config value from a non-value token.");
    }

}  // namespace hocon
