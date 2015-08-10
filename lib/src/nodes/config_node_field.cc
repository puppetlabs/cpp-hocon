#include <internal/nodes/config_node_field.hpp>
#include <internal/config_exception.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/tokens.hpp>
#include <internal/nodes/config_node_comment.hpp>

using namespace std;

namespace hocon {

    config_node_field::config_node_field(vector<shared_ptr<abstract_config_node>> children) :
            _children(move(children)) { }

    vector<shared_ptr<token>> config_node_field::get_tokens() const {
        vector<shared_ptr<token>> tokens;
        for (auto&& node : _children) {
            for (auto&& token : node->get_tokens()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    shared_ptr<config_node_field> config_node_field::replace_value(shared_ptr<abstract_config_node_value> new_value) {
        vector<shared_ptr<abstract_config_node>> children_copy;
        bool replaced = false;
        for (auto&& child : _children) {
            if(dynamic_pointer_cast<abstract_config_node_value>(child)) {
                children_copy.push_back(new_value);
                replaced = true;
            } else {
                children_copy.push_back(child);
            }
        }
        if (replaced) {
            return make_shared<config_node_field>(children_copy);
        } else {
            throw config_exception("Field node doesn't have a value.");
        }
    }

    shared_ptr<abstract_config_node_value> config_node_field::get_value() const {
        for (auto&& child : _children) {
            shared_ptr<abstract_config_node_value> value =
                    dynamic_pointer_cast<abstract_config_node_value>(child);
            if(value) {
                return value;
            }
        }
        throw config_exception("Field node doesn't have a value.");
    }

    // TODO: Implement path() once we have config_node_path

    shared_ptr<token> config_node_field::separator() const {
        for (auto&& child : _children) {
            shared_ptr<config_node_single_token> single_token =
                    dynamic_pointer_cast<config_node_single_token>(child);
            if (single_token) {
                shared_ptr<token> t = single_token->get_token();
                if (t == tokens::plus_equals_token() || t == tokens::colon_token() || t == tokens::equals_token()) {
                    return t;
                }
            }
        }
        return nullptr;
     }

    vector<string> config_node_field::comments() const {
        vector<string> comments;
        for (auto&& child : _children) {
            shared_ptr<config_node_comment> comment = dynamic_pointer_cast<config_node_comment>(child);
            if (comment) {
                comments.push_back(comment->comment_text());
            }
        }
        return comments;
    }

}  // namespace hocon
