#pragma once

#include "types.hpp"
#include <string>
#include <memory>
#include "export.h"

namespace hocon {
    /**
     * Implement this interface and provide an instance to
     * {@link config_parse_options#set_includer config_parse_options.set_includer()} to
     * customize handling of {@code include} statements in config files. You may
     * also want to implement {@link config_includer_file} and {@link config_includer_URL}, or not.
     */
    class LIBCPP_HOCON_EXPORT config_includer {
    public:
        /**
         * Returns a new includer that falls back to the given includer. This is how
         * you can obtain the default includer; it will be provided as a fallback.
         * It's up to your includer to chain to it if you want to. You might want to
         * merge any files found by the fallback includer with any objects you load
         * yourself.
         *
         * It's important to handle the case where you already have the fallback
         * with a "return this", i.e. this method should not create a new object if
         * the fallback is the same one you already have. The same fallback may be
         * added repeatedly.
         *
         * @param fallback the previous includer for chaining
         * @return a new includer
         */
        virtual shared_includer with_fallback(shared_includer fallback) const = 0;

        /**
         * Parses another item to be included. The returned object typically would
         * not have substitutions resolved. You can throw a config_exception here to
         * abort parsing, or return an empty object, but may not return null.
         * 
         * This method is used for a "heuristic" include statement that does not
         * specify file, or URL resource. If the include statement does
         * specify, then the same class implementing {@link config_includer} must
         * also implement {@link config_includer_file} or {@link config_includer_URL} as needed, or a
         * default includer will be used.
         * 
         * @param context
         *            some info about the include context
         * @param what
         *            the include statement's argument
         * @return a non-null config_object
         */
        virtual shared_object include(shared_include_context context, std::string what) const = 0;
    };
}  // namespace hocon
