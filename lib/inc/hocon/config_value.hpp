#pragma once

#include "config_origin.hpp"
#include "config_render_options.hpp"
#include "path.hpp"
#include <string>
#include "export.h"

namespace hocon {

    /**
     * The type of a configuration value (following the <a
     * href="http://json.org">JSON</a> type schema).
     */
    enum class config_value_type {
        OBJECT, LIST, NUMBER, BOOLEAN, CONFIG_NULL, STRING, UNSPECIFIED
    };

    enum class resolve_status { RESOLVED, UNRESOLVED };

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
    class LIBCPP_HOCON_EXPORT config_value : public std::enable_shared_from_this<config_value> {
        friend class token;
        friend class value;
        friend class default_transformer;
        friend class config;
        friend class simple_config_object;
    public:
        /**
         * The origin of the value (file, line number, etc.), for debugging and
         * error messages.
         *
         * @return where the value came from
         */
        virtual shared_origin const& origin() const;

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
        virtual std::string render() const;

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
        virtual std::string render(config_render_options options) const;

        /**
         * Places the value inside a {@link config} at the given key. See also
         * {@link config_value#at_path(string)}.
         *
         * @param key
         *            key to store this value at.
         * @return a {@code config} instance containing this value at the given key.
         */
        shared_config at_key(std::string const& key) const;

        /**
         * Places the value inside a {@link Config} at the given path. See also
         * {@link ConfigValue#atKey(String)}.
         *
         * @param path
         *            path to store this value at.
         * @return a {@code Config} instance containing this value at the given
         *         path.
         */
        shared_config at_path(std::string const& path_expression) const;

    protected:
        config_value(shared_origin origin);

        virtual std::string transform_to_string() const;
        virtual resolve_status get_resolve_status() const;
        void render(std::string& result, int indent, bool at_root, std::string at_key,
                    config_render_options options) const;
        virtual void render(std::string& result, int indent, bool at_root, config_render_options options) const;

        shared_config at_key(shared_origin origin, std::string const& key) const;
        shared_config at_path(shared_origin origin, path raw_path) const;

    private:
        shared_origin _origin;
    };

}  // namespace hocon
