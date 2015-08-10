#include <internal/nodes/config_node_complex_value.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/tokens.hpp>
#include <internal/nodes/config_node_field.hpp>

using namespace std;

namespace hocon {

    config_node_complex_value::config_node_complex_value(vector<shared_ptr<abstract_config_node>> children) :
            _children(move(children)) { }

    vector<shared_ptr<abstract_config_node>> const& config_node_complex_value::children() const {
        return _children;
    }

    vector<shared_ptr<token>> config_node_complex_value::get_tokens() const {
        vector<shared_ptr<token>> tokens;
        for(auto&& node : _children) {
            for(auto&& token : node->get_tokens()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    shared_ptr<config_node_complex_value> config_node_complex_value::indent_text(
            shared_ptr<abstract_config_node> indentation)
    {
        vector<shared_ptr<abstract_config_node>> children_copy;
        for (auto&& child : _children) {
            shared_ptr<config_node_single_token> single_token =
                    dynamic_pointer_cast<config_node_single_token>(child);
            if (single_token) {
                shared_ptr<line> new_line = dynamic_pointer_cast<line>(single_token->get_token());
                if (new_line) {
                    children_copy.push_back(child);
                    children_copy.push_back(indentation);
                    continue;
                }
            }

            shared_ptr<config_node_complex_value> complex =
                    dynamic_pointer_cast<config_node_complex_value>(child);
            if (complex) {
                children_copy.push_back(complex->indent_text(indentation));
                continue;
            }

            shared_ptr<config_node_field> field = dynamic_pointer_cast<config_node_field>(child);
            if (field) {
                shared_ptr<abstract_config_node_value> value = field->get_value();
                shared_ptr<config_node_complex_value> complex_node =
                        dynamic_pointer_cast<config_node_complex_value>(value);
                if (complex_node) {
                    children_copy.push_back(field->replace_value(complex_node->indent_text(indentation)));
                    continue;
                }
            }
        }
        return new_node(children_copy);
    }

}  // namespace hocon
