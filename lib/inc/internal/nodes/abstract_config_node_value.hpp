#pragma once

#include <internal/abstract_config_node.hpp>

namespace hocon {

    /**
     * This is required if we want
     * to be referencing the AbstractConfigNode class in implementation rather than the
     * ConfigNode interface, as we can't cast an AbstractConfigNode to an interface
     * TODO: this may or may not be necessary in C++, will have to see
     */
    class abstract_config_node_value : public abstract_config_node {

    };

}  // namespace hocon