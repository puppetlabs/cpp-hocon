#pragma once

#include <hocon/config_value.hpp>
namespace hocon {

    /**
     * An Abstractconfig_value which contains other values. Java has no way to
     * express "this has to be an Abstractconfig_value also" other than making
     * Abstractconfig_value an interface which would be aggravating. But we can say
     * we are a config_value.
     */
    class container {
    public:
        /**
         * Replace a child of this value. CAUTION if replacement is null, delete the
         * child, which may also delete the parent, or make the parent into a
         * non-container.
         */
        virtual shared_value replace_child(shared_value const& child, shared_value replacement) const = 0;

        /**
         * Super-expensive full traversal to see if descendant is anywhere
         * underneath this container.
         */
        virtual bool has_descendant(shared_value const& descendant) const = 0;
    };

}  // namespace hocon
