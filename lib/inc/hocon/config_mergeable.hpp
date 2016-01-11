#pragma once

#include <memory>
#include "export.h"

namespace hocon {

    class LIBCPP_HOCON_EXPORT config_mergeable {
        friend class config_value;
    public:
        /**
         * Returns a new value computed by merging this value with another, with
         * keys in this value "winning" over the other one.
         *
         * <p>
         * This associative operation may be used to combine configurations from
         * multiple sources (such as multiple configuration files).
         *
         * <p>
         * The semantics of merging are described in the <a
         * href="https://github.com/typesafehub/config/blob/master/HOCON.md">spec
         * for HOCON</a>. Merging typically occurs when either the same object is
         * created twice in the same file, or two config files are both loaded. For
         * example:
         *
         * <pre>
         *  foo = { a: 42 }
         *  foo = { b: 43 }
         * </pre>
         *
         * Here, the two objects are merged as if you had written:
         *
         * <pre>
         *  foo = { a: 42, b: 43 }
         * </pre>
         *
         * <p>
         * Only {@link ConfigObject} and {@link Config} instances do anything in
         * this method (they need to merge the fallback keys into themselves). All
         * other values just return the original value, since they automatically
         * override any fallback. This means that objects do not merge "across"
         * non-objects; if you write
         * <code>object.withFallback(nonObject).withFallback(otherObject)</code>,
         * then <code>otherObject</code> will simply be ignored. This is an
         * intentional part of how merging works, because non-objects such as
         * strings and integers replace (rather than merging with) any prior value:
         *
         * <pre>
         * foo = { a: 42 }
         * foo = 10
         * </pre>
         *
         * Here, the number 10 "wins" and the value of <code>foo</code> would be
         * simply 10. Again, for details see the spec.
         *
         * @param other
         *            an object whose keys should be used as fallbacks, if the keys
         *            are not present in this one
         * @return a new object (or the original one, if the fallback doesn't get
         *         used)
         */
        virtual std::shared_ptr<const config_mergeable> with_fallback(std::shared_ptr<const config_mergeable> other) const = 0;

    protected:
        /**
         * Converts a config to its root object and a config_value to itself.
         * Originally in the MergeableValue interface, squashing to ease C++ public API separation
         */
        virtual shared_value to_fallback_value() const = 0;
    };

}  // namespace hocon
