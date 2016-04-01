#include <internal/nodes/config_node_array.hpp>

using namespace std;

namespace hocon {

    config_node_array::config_node_array(shared_node_list children) : config_node_complex_value(move(children)) { }

    shared_ptr<const config_node_complex_value> config_node_array::new_node(shared_node_list nodes) const {
        return make_shared<config_node_array>(nodes);
    }

}  // namespace hocon
