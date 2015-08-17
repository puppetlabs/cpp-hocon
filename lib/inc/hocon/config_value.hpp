#pragma once

#include <string>
#include "config_render_options.hpp"

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

        /**
         * Renders the config value as a HOCON string. This method is primarily
         * intended for debugging, so it tries to add helpful comments and
         * whitespace.
         *
         * <p>
         * If the config value has not been resolved (see {@link config#resolve}),
         * it's possible that it can't be rendered as valid HOCON. In that case the
         * rendering should still be useful for debugging but you might not be able
         * to parse it. If the value has been resolved, it will always be parseable.
         *
         * <p>
         * This method is equivalent to
         * {@code render(config_render_options())}.
         *
         * @return the rendered value
         */
        virtual std::string render() const = 0;

        /**
         * Renders the config value to a string, using the provided options.
         *
         * <p>
         * If the config value has not been resolved (see {@link config#resolve}),
         * it's possible that it can't be rendered as valid HOCON. In that case the
         * rendering should still be useful for debugging but you might not be able
         * to parse it. If the value has been resolved, it will always be parseable.
         *
         * <p>
         * If the config value has been resolved and the options disable all
         * HOCON-specific features (such as comments), the rendering will be valid
         * JSON. If you enable HOCON-only features such as comments, the rendering
         * will not be valid JSON.
         *
         * @param options
         *            the rendering options
         * @return the rendered value
         */
        virtual std::string render(config_render_options options) const = 0;
    };
}  // namespace hocon
