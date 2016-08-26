#include <internal/nodes/config_node_field.hpp>
#include <hocon/config_exception.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/tokens.hpp>
#include <internal/nodes/config_node_comment.hpp>
#include <internal/nodes/config_node_path.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_node_field::config_node_field(shared_node_list children) :
            _children(move(children)) { }

    token_list config_node_field::get_tokens() const {
        token_list tokens;
        for (auto&& node : _children) {
            token_list node_tokens = node->get_tokens();
            tokens.insert(tokens.end(), node_tokens.begin(), node_tokens.end());
        }
        return tokens;
    }

    shared_ptr<const config_node_field> config_node_field::replace_value(shared_node_value new_value) const {
        shared_node_list children_copy = _children;
        for (size_t i = 0; i < children_copy.size(); i++) {
            if (dynamic_pointer_cast<const abstract_config_node_value>(children_copy[i])) {
                children_copy[i] = new_value;
                return make_shared<config_node_field>(move(children_copy));
            }
        }
        throw config_exception(_("Field doesn't have a value."));
    }

    shared_node_value config_node_field::get_value() const {
        for (auto&& child : _children) {
            if (auto value = dynamic_pointer_cast<const abstract_config_node_value>(child)) {
                return value;
            }
        }
        throw config_exception(_("Field node doesn't have a value."));
    }

    shared_ptr<const config_node_path> config_node_field::path() const {
        for (auto&& node : _children) {
            if (auto path_node = dynamic_pointer_cast<const config_node_path>(node)) {
                return path_node;
            }
        }
        throw config_exception(_("Field node does not have a path"));
    }

    shared_token config_node_field::separator() const {
        for (auto&& child : _children) {
            if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(child)) {
                shared_token t = single_token->get_token();
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
            auto comment = dynamic_pointer_cast<const config_node_comment>(child);
            if (comment) {
                comments.push_back(comment->comment_text());
            }
        }
        return comments;
    }

}  // namespace hocon
