#pragma once

#include "types.hpp"
#include "export.h"

namespace hocon {

    /**
     * Implement this interface and provide an instance to
     * {@link ConfigResolveOptions#appendResolver ConfigResolveOptions.appendResolver()}
     * to provide custom behavior when unresolved substitutions are encountered
     * during resolution.
     * @since 1.3.2
     */
    class LIBCPP_HOCON_EXPORT config_resolver {
    public:
        /**
         * Returns the value to substitute for the given unresolved path. To get the
         * components of the path use {@link ConfigUtil#splitPath(String)}. If a
         * non-null value is returned that value will be substituted, otherwise
         * resolution will continue to consider the substitution as still
         * unresolved.
         *
         * @param path the unresolved path
         * @return the value to use as a substitution or null
         */
        shared_value lookup(const std::string &path) const
        {
          return nullptr;
        }

        /**
         * Returns a new resolver that falls back to the given resolver if this
         * one doesn't provide a substitution itself.
         *
         * It's important to handle the case where you already have the fallback
         * with a "return this", i.e. this method should not create a new object if
         * the fallback is the same one you already have. The same fallback may be
         * added repeatedly.
         *
         * @param fallback the previous includer for chaining
         * @return a new resolver
         */
        shared_resolver with_fallback(shared_resolver fallback) const
        {
          return fallback;
        }
    };
}  // namespace hocon
