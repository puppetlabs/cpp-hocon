#pragma once

#include <hocon/config_value.hpp>
#include <vector>

namespace hocon {

    /**
    * Interface that tags a config_value that is not mergeable until after
    * substitutions are resolved. Basically these are special config_value that
    * never appear in a resolved tree, like {@link config_substitution} and
    * {@link config_delayed_merge}.
    */
    class unmergeable {
    public:
        virtual std::vector<shared_value> unmerged_values() const = 0;
    };

}  // namespace hocon

