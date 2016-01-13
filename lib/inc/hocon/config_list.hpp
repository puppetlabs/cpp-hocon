#pragma once

#include "config_value.hpp"
#include "config_origin.hpp"
#include <vector>
#include "export.h"

namespace hocon {

    /**
     * Subtype of {@link ConfigValue} representing a list value, as in JSON's
     * {@code [1,2,3]} syntax.
     *
     * <p>
     * {@code ConfigList} implements {@code java.util.List<ConfigValue>} so you can
     * use it like a regular Java list. Or call {@link #unwrapped()} to unwrap the
     * list elements into plain Java values.
     *
     * <p>
     * Like all {@link ConfigValue} subtypes, {@code ConfigList} is immutable. This
     * makes it threadsafe and you never have to create "defensive copies." The
     * mutator methods from {@link java.util.List} all throw
     * {@link java.lang.UnsupportedOperationException}.
     *
     * <p>
     * The {@link ConfigValue#valueType} method on a list returns
     * {@link ConfigValueType#LIST}.
     *
     * <p>
     * <em>Do not implement {@code ConfigList}</em>; it should only be implemented
     * by the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     *
     */

    class LIBCPP_HOCON_EXPORT config_list : public config_value {
    public:
        config_list(shared_origin origin) : config_value(move(origin)) {}
    };
}  // namespace hocon
