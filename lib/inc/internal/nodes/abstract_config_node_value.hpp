#pragma once

#include "abstract_config_node.hpp"

namespace hocon {

    /** This is used to classify certain abstract_config_node subclasses. */
    class abstract_config_node_value : public abstract_config_node { };

    using shared_node_value = std::shared_ptr<const abstract_config_node_value>;

}  // namespace hocon
