#pragma once

#include <string>

namespace hocon {
    /**
     * The type of a configuration value (following the <a
     * href="http://json.org">JSON</a> type schema).
     */
    enum class config_value_type {
        OBJECT, LIST, NUMBER, BOOLEAN, CONFIG_NULL, STRING
    };

    /**
     * An immutable value, following the <a href="http://json.org">JSON</a> type
     * schema.
     *
     * <p>
     * Because this object is immutable, it is safe to use from multiple threads and
     * there's no need for "defensive copies."
     *
     * <p>
     * <em>Do not implement interface {@code ConfigValue}</em>; it should only be
     * implemented by the config library. Arbitrary implementations will not work
     * because the library internals assume a specific concrete implementation.
     * Also, this interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class config_value {
    public:
        /**
         * The config_value_type of the value; matches the JSON type schema.
         *
         * @return value's type
         */
        virtual config_value_type value_type() const = 0;
    };
}  // namespace hocon
