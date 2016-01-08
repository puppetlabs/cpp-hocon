#pragma once

#include "config_parse_options.hpp"
#include "config_object.hpp"
#include "config_origin.hpp"
#include "parser/config_document.hpp"
#include <memory>
#include "export.h"

namespace hocon {

    /**
     * An opaque handle to something that can be parsed, obtained from
     * {@link config_include_context}.
     *
     * <p>
     * <em>Do not implement this interface</em>; it should only be implemented by
     * the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class LIBCPP_HOCON_EXPORT config_parseable {
    public:
        /**
         * Parse whatever it is. The options should come from
         * {@link config_parseable#options()} but you could tweak them if you
         * like.
         *
         * @param options
         *            parse options, should be based on the ones from
         *            {@link config_parseable#options()}
         * @return the parsed object
         */
        // TODO: This relies on the ConfigParser, which has a separate ticket
        // virtual std::shared_ptr<config_object> parse(shared_parse_options options) = 0;

        /**
         * Returns a config_origin describing the origin of the paresable item.
         */
        virtual config_origin origin() const = 0;

        /**
         * Get the initial options, which can be modified then passed to parse().
         * These options will have the right description, includer, and other
         * parameters already set up.
         * @return the initial options
         */
        virtual shared_parse_options options() const = 0;
    };

}  // namespace hocon
