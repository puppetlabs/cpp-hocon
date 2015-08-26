#pragma once

#include <memory>
#include <vector>

namespace hocon {

    class config;
    using shared_config = std::shared_ptr<const config>;

    class config_object;
    using shared_object = std::shared_ptr<const config_object>;

    class config_origin;
    using shared_origin = std::shared_ptr<const config_origin>;

    class path;

    class config_value;
    using shared_value = std::shared_ptr<const config_value>;

    class abstract_config_node;
    using shared_node = std::shared_ptr<abstract_config_node>;
    using shared_node_list = std::vector<shared_node>;

}  // namespace hocon
