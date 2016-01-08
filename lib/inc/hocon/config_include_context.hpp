#pragma once

#include "config_parseable.hpp"
#include "config_parse_options.hpp"
#include "export.h"

namespace hocon {

    class parseable;

    /**
     * Context provided to a {@link config_includer}; this interface is only useful
     * inside a {@code config_includer} implementation, and is not intended for apps
     * to implement.
     *
     * <p>
     * <em>Do not implement this interface</em>; it should only be implemented by
     * the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class config_include_context {
    public:
        //-------------------- PUBLIC API --------------------
        /**
         * Tries to find a name relative to whatever is doing the including, for
         * example in the same directory as the file doing the including. Returns
         * null if it can't meaningfully create a relative name. The returned
         * parseable may not exist; this function is not required to do any IO, just
         * compute what the name would be.
         *
         * The passed-in filename has to be a complete name (with extension), not
         * just a basename. (Include statements in config files are allowed to give
         * just a basename.)
         *
         * @param filename
         *            the name to make relative to the resource doing the including
         * @return parseable item relative to the resource doing the including, or
         *         null
         */
        LIBCPP_HOCON_EXPORT std::shared_ptr<config_parseable> relative_to(std::string file_name) const;

        /**
         * Parse options to use (if you use another method to get a
         * {@link config_parseable} then use {@link config_parseable#options()}
         * instead though).
         *
         * @return the parse options
         */
        LIBCPP_HOCON_EXPORT config_parse_options parse_options() const;

        //-------------------- INTERNAL API --------------------
        config_include_context(std::shared_ptr<parseable> parseable);

    private:
        std::shared_ptr<parseable> _parseable;
    };

}  // namespace hocon
