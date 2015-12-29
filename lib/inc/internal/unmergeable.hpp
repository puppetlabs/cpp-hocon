#pragma once

#include <hocon/config_value.hpp>
#include <vector>

namespace hocon {

    /**
    * Interface that tags a ConfigValue that is not mergeable until after
    * substitutions are resolved. Basically these are special ConfigValue that
    * never appear in a resolved tree, like {@link ConfigSubstitution} and
    * {@link ConfigDelayedMerge}.
    */
    class unmergeable {
    public:
        virtual std::vector<shared_value> unmerged_values() const = 0;
    };

}  // namespace hocon

