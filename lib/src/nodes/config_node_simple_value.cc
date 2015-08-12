#include <internal/nodes/config_node_simple_value.hpp>
#include <internal/config_exception.hpp>
#include <internal/tokens.hpp>
#include <internal/values/config_string.hpp>

using namespace std;

namespace hocon {

    config_node_simple_value::config_node_simple_value(shared_token value) : _value(move(value)) { }

    shared_token config_node_simple_value::get_token() const {
        return _value;
    }

    token_list config_node_simple_value::get_tokens() const {
        return token_list { _value };
    }

    unique_ptr<abstract_config_value> const& config_node_simple_value::get_value() const {
        shared_ptr<value> value_token = dynamic_pointer_cast<value>(_value);
        if (value_token) {
            return value_token->get_value();
        }

        // TODO: We have two separate cases here: in the above, we have an existing value
        // but below, we need to create new ones. What should this method return in order to
        // support both?
//        shared_ptr<unquoted_text> text_token = dynamic_pointer_cast<unquoted_text>(_value);
//        if (text_token) {
//            return unique_ptr<config_string>(new config_string(
//                    text_token->origin(), text_token->token_text(), config_string_type::UNQUOTED));
//        }
//
//        shared_ptr<substitution> sub_token = dynamic_pointer_cast<substitution>(_value);
//        if(sub_token) {
//            token_list expression = sub_token->expression();
              // TODO: this will require Path and ConfigReference to be ported to handle properly
//        }

        throw config_exception("Tried to get a config value from a non-value token.");
    }

}  // namespace hocon
