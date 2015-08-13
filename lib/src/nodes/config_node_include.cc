#include <internal/nodes/config_node_include.hpp>
#include <internal/nodes/config_node_simple_value.hpp>

using namespace std;

namespace hocon {

    config_node_include::config_node_include(vector<unique_ptr<abstract_config_node>> children,
                                             config_include_kind kind) :
        _children(move(children)), _kind(kind) { }

    token_list config_node_include::get_tokens() const {
        token_list tokens;
        for (auto&& node : _children) {
            tokens.insert(tokens.end(), node->get_tokens().begin(), node->get_tokens().end());
        }
        return tokens;
    }

    vector<unique_ptr<abstract_config_node>> const& config_node_include::children() const {
        return _children;
    }

    config_include_kind config_node_include::kind() const {
        return _kind;
    }

    string config_node_include::name() const {
        for (auto&& node : _children) {
            if (config_node_simple_value* simple = dynamic_cast<config_node_simple_value*>(node.get())) {
                return simple->get_value()->transform_to_string();
            }
        }
        return "";
    }

}  // namesapce hocon
