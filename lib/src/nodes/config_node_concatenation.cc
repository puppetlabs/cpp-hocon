#include <internal/nodes/config_node_concatenation.hpp>

using namespace std;

namespace hocon {

    config_node_concatenation::config_node_concatenation(shared_node_list children) :
            config_node_complex_value(move(children)) { }

    shared_ptr<const config_node_complex_value> config_node_concatenation::new_node(shared_node_list nodes) const {
        return make_shared<config_node_concatenation>(nodes);
    }

}  // namespace hoocon
