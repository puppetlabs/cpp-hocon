#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
#if defined(__GNUC__) && __GNUC__ > 5
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
#include "boost/variant.hpp"
#pragma GCC diagnostic pop

namespace hocon {

    /**
     * A duration represented as a 64-bit integer of seconds plus
     * a 32-bit number of nanoseconds representing a fraction of a
     * second.
     */
    using duration = std::pair<int64_t, int>;

    class config;
    using shared_config = std::shared_ptr<const config>;

    class config_object;
    using shared_object = std::shared_ptr<const config_object>;

    class config_origin;
    using shared_origin = std::shared_ptr<const config_origin>;

    class path;

    class config_value;
    using shared_value = std::shared_ptr<const config_value>;

    class config_list;
    using shared_list = std::shared_ptr<const config_list>;

    typedef boost::make_recursive_variant<boost::blank, std::string, int64_t, double, int, bool,
            std::vector<boost::recursive_variant_>, std::unordered_map<std::string,
                    boost::recursive_variant_>>::type unwrapped_value;

    class container;
    using shared_container = std::shared_ptr<const container>;

    class abstract_config_node;
    using shared_node = std::shared_ptr<const abstract_config_node>;
    using shared_node_list = std::vector<shared_node>;

    using shared_string = std::shared_ptr<const std::string>;

    class config_parse_options;

    class config_includer;
    using shared_includer = std::shared_ptr<const config_includer>;

    class config_include_context;
    using shared_include_context = std::shared_ptr<const config_include_context>;

    class config_parseable;
    using shared_parseable = std::shared_ptr<const config_parseable>;

    class full_path_operator;
    using shared_full_current = std::shared_ptr<full_path_operator>;
}  // namespace hocon
