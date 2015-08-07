#include <internal/nodes/config_node_single_token.hpp>

using namespace std;

namespace hocon {

    config_node_single_token::config_node_single_token(shared_ptr<token> t) :
            _token(move(t)) { }

    vector<shared_ptr<token>> config_node_single_token::get_tokens() const {
        return vector<shared_ptr<token>> { _token };
    }

    shared_ptr<token> config_node_single_token::get_token() const {
        return _token;
    }

}  // namespace hocon
