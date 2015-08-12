#include <internal/nodes/config_node_single_token.hpp>

using namespace std;

namespace hocon {

    config_node_single_token::config_node_single_token(shared_token t) :
            _token(move(t)) { }

    token_list config_node_single_token::get_tokens() const {
        return { _token };
    }

    shared_token config_node_single_token::get_token() const {
        return _token;
    }

}  // namespace hocon
