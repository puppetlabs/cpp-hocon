#pragma once

#include "export.h"

namespace hocon {

    /**
     * A set of options related to resolving substitutions. Substitutions use the
     * <code>${foo.bar}</code> syntax and are documented in the <a
     * href="https://github.com/typesafehub/config/blob/master/HOCON.md">HOCON</a>
     * spec.
     * <p>
     * Typically this class would be used with the method
     * {@link config#resolve(config_resolve_options)}.
     * <p>
     * This object is immutable, so the "setters" return a new object.
     * <p>
     * Here is an example of creating a custom {@code config_resolve_options}:
     *
     * <pre>
     *     config_resolve_options options = config_resolve_options()
     *         .set_use_system_environment(false)
     * </pre>
     * <p>
     * In addition to {@link config_resolve_options}, there's a prebuilt
     * {@link config_resolve_options#no_system} which avoids looking at any system
     * environment variables or other external system information. (Right now,
     * environment variables are the only example.)
     */
    class LIBCPP_HOCON_EXPORT config_resolve_options {
    public:
        /**
         * Returns the default resolve options. By default the system environment
         * will be used and unresolved substitutions are not allowed.
         *
         * @return the default resolve options
         */
        config_resolve_options(bool use_system_environment = true, bool allow_unresolved = false);

        /**
         * Returns resolve options that disable any reference to "system" data
         * (currently, this means environment variables).
         *
         * @return the resolve options with env variables disabled
         */
        config_resolve_options set_use_system_environment(bool value) const;

        /**
         * Returns whether the options enable use of system environment variables.
         * This method is mostly used by the config lib internally, not by
         * applications.
         *
         * @return true if environment variables should be used
         */
        bool get_use_system_environment() const;

        /**
         * Returns options with "allow unresolved" set to the given value. By
         * default, unresolved substitutions are an error. If unresolved
         * substitutions are allowed, then a future attempt to use the unresolved
         * value may fail, but {@link config#resolve(config_resolve_options)} itself
         * will not throw.
         *
         * @param value
         *            true to silently ignore unresolved substitutions.
         * @return options with requested setting for whether to allow substitutions
         */
        config_resolve_options set_allow_unresolved(bool value) const;

        /**
         * Returns whether the options allow unresolved substitutions. This method
         * is mostly used by the config lib internally, not by applications.
         *
         * @return true if unresolved substitutions are allowed
         */
        bool get_allow_unresolved() const;

    private:
        bool _use_system_environment;
        bool _allow_unresovled;
    };

}  // namespace hocon
