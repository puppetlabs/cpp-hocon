#pragma once

#include "types.hpp"
#include "export.h"
#include <vector>

namespace hocon {

    /**
     * Subtype of {@link config_value} representing a list value, as in JSON's
     * {@code [1,2,3]} syntax.
     *
     * <p>
     * {@code config_list} implements {@code java.util.List<config_value>} so you can
     * use it like a regular Java list. Or call {@link #unwrapped()} to unwrap the
     * list elements into plain Java values.
     *
     * <p>
     * Like all {@link config_value} subtypes, {@code config_list} is immutable. This
     * makes it threadsafe and you never have to create "defensive copies." The
     * mutator methods from {@link java.util.List} all throw
     * {@link java.lang.UnsupportedOperationException}.
     *
     * <p>
     * The {@link config_value#valueType} method on a list returns
     * {@link config_valueType#LIST}.
     *
     * <p>
     * <em>Do not implement {@code config_list}</em>; it should only be implemented
     * by the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     *
     */

    class LIBCPP_HOCON_EXPORT config_list : public config_value {
    public:
        config_list(shared_origin origin) : config_value(move(origin)) {}

        // list interface
        using iterator = std::vector<shared_value>::const_iterator;
        virtual bool is_empty() const = 0;
        virtual size_t size() const = 0;
        virtual shared_value operator[](size_t index) const = 0;
        virtual shared_value get(size_t index) const = 0;
        virtual iterator begin() const = 0;
        virtual iterator end() const = 0;
        virtual unwrapped_value unwrapped() const = 0;
    };
}  // namespace hocon
