#include <internal/nodes/config_node_complex_value.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/tokens.hpp>
#include <internal/nodes/config_node_field.hpp>

using namespace std;

namespace hocon {

    config_node_complex_value::config_node_complex_value(shared_node_list children) :
            _children(move(children)) { }

    shared_node_list const& config_node_complex_value::children() const {
        return _children;
    }

    token_list config_node_complex_value::get_tokens() const {
        token_list tokens;
        int i = 0;
        for (auto&& node : _children) {
            i++;
            token_list node_tokens = node->get_tokens();
            tokens.insert(tokens.end(), node_tokens.begin(), node_tokens.end());
        }
        return tokens;
    }

    shared_ptr<const config_node_complex_value> config_node_complex_value::indent_text(
            shared_node indentation) const
    {
        shared_node_list children_copy = _children;
        for (size_t i = 0; i < children_copy.size(); i++) {
            auto child = children_copy[i];
            if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(child)) {
                if (single_token->get_token()->get_token_type() == token_type::NEWLINE) {
                    children_copy.insert(children_copy.begin() + i + 1, indentation);
                }
            } else if (auto field = dynamic_pointer_cast<const config_node_field>(child)) {
                auto value = field->get_value();
                if (auto complex = dynamic_pointer_cast<const config_node_complex_value>(value)) {
                    children_copy[i] = field->replace_value(complex->indent_text(indentation));
                }
            } else if (auto complex = dynamic_pointer_cast<const config_node_complex_value>(child)) {
                children_copy[i] = complex->indent_text(indentation);
            }
        }
        return new_node(children_copy);
    }

}  // namespace hocon
