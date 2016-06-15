#include <internal/nodes/config_node_include.hpp>
#include <internal/nodes/config_node_simple_value.hpp>

using namespace std;

namespace hocon {

    config_node_include::config_node_include(shared_node_list children,
                                             config_include_kind kind) :
        _children(move(children)), _kind(kind) { }

    token_list config_node_include::get_tokens() const {
        token_list tokens;
        for (auto&& node : _children) {
            token_list node_tokens = node->get_tokens();
            tokens.insert(tokens.end(), node_tokens.begin(), node_tokens.end());
        }
        return tokens;
    }

    shared_node_list const& config_node_include::children() const {
        return _children;
    }

    config_include_kind config_node_include::kind() const {
        return _kind;
    }

    string config_node_include::name() const {
        for (auto&& node : _children) {
            if (auto simple = dynamic_cast<const config_node_simple_value*>(node.get())) {
                return simple->get_value()->transform_to_string();
            }
        }
        return "";
    }

}  // namesapce hocon
