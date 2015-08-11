#include <internal/nodes/config_node_path.hpp>

using namespace std;

namespace hocon {

    config_node_path::config_node_path(path node_path, token_list tokens) :
        _path(move(node_path)), _tokens(move(tokens)) { }

    token_list config_node_path::get_tokens() const {
        return _tokens;
    }

    path config_node_path::get_path() const {
        return _path;
    }
}  // namespace hocon
