#include <internal/nodes/config_node_path.hpp>
#include <hocon/config_exception.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

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

    config_node_path config_node_path::sub_path(int to_remove) {
        int period_count = 0;
        token_list tokens_copy = _tokens;
        for (auto it = tokens_copy.begin(); it != tokens_copy.end(); it++) {
            if ((*it)->get_token_type() == token_type::UNQUOTED_TEXT && (*it)->token_text() == ".") {
                period_count++;
            }

            if (period_count == to_remove) {
                return config_node_path(_path.sub_path(to_remove), token_list { it + 1, tokens_copy.end() });
            }
        }
        throw config_exception(_("Tried to remove too many elements from a path node."));
    }

    config_node_path config_node_path::first() {
        token_list tokens_copy = _tokens;
        for (auto it = tokens_copy.begin(); it != tokens_copy.end(); it++) {
            if ((*it)->get_token_type() == token_type::UNQUOTED_TEXT && (*it)->token_text() == ".") {
                return config_node_path(_path.sub_path(0, 1), token_list { tokens_copy.begin(), it });
            }
        }
        return *this;
    }
}  // namespace hocon
