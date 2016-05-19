#pragma once

#include "types.hpp"
#include "config_syntax.hpp"
#include "export.h"

namespace hocon {
    /**
     * A set of options related to parsing.
     *
     * <p>
     * This object is immutable, so the "setters" return a new object.
     *
     * <p>
     * Here is an example of creating a custom {@code config_parse_options}:
     *
     * <pre>
     *     config_parse_options options = config_parse_options()
     *         .set_syntax(config_syntax.JSON)
     *         .set_allow_missing(false)
     * </pre>
     *
     * ClassLoader is Java-specific, so it was not ported to C++.
     */
    class LIBCPP_HOCON_EXPORT config_parse_options {
    public:
        /**
         * Gets an instance of <code>config_parse_options</code> with all fields
         * set to the default values. Start with this instance and make any
         * changes you need.
         * @return the default parse options
         */
        config_parse_options();

        /**
         * Gets an instance of <code>ConfigParseOptions</code> with all fields
         * set to the default values. Start with this instance and make any
         * changes you need.
         * @return the default parse options
         */
        static config_parse_options defaults();

        /**
         * Set the file format. If set to null, try to guess from any available
         * filename extension; if guessing fails, assume {@link config_syntax#CONF}.
         *
         * @param syntax
         *            a syntax or {@code nullptr} for best guess
         * @return options with the syntax set
         */
        config_parse_options set_syntax(config_syntax syntax) const;

        /**
         * Gets the current syntax option
         */
        config_syntax const& get_syntax() const;

        /**
         * Set a description for the thing being parsed. In most cases this will be
         * set up for you to something like the filename, but if you provide just an
         * input stream you might want to improve on it. Set to null to allow the
         * library to come up with something automatically. This description is the
         * basis for the {@link config_origin} of the parsed values.
         *
         * @param origin_description description to put in the {@link config_origin}
         * @return options with the origin description set
         */
        config_parse_options set_origin_description(shared_string origin_description) const;

        /**
         * Gets the current origin description, which may be null for "automatic".
         * @return the current origin description or null
         */
        shared_string const& get_origin_description() const;

        /**
         * Set to false to throw an exception if the item being parsed (for example
         * a file) is missing. Set to true to just return an empty document in that
         * case.
         *
         * @param allow_missing true to silently ignore missing item
         * @return options with the "allow missing" flag set
         */
        config_parse_options set_allow_missing(bool allow_missing) const;

        /**
         * Gets the current "allow missing" flag.
         * @return whether we allow missing files
         */
        bool get_allow_missing() const;

        /**
         * Set a {@link config_includer} which customizes how includes are handled.
         * null means to use the default includer.
         *
         * @param includer the includer to use or null for default
         * @return new version of the parse options with different includer
         */
        config_parse_options set_includer(shared_includer includer) const;

        /**
         * Prepends a {@link config_includer} which customizes how
         * includes are handled.  To prepend your includer, the
         * library calls {@link config_includer#with_fallback} on your
         * includer to append the existing includer to it.
         *
         * @param includer the includer to prepend (may not be null)
         * @return new version of the parse options with different includer
         */
        config_parse_options prepend_includer(shared_includer includer) const;

        /**
         * Appends a {@link config_includer} which customizes how
         * includes are handled.  To append, the library calls {@link
         * config_includer#with_fallback} on the existing includer.
         *
         * @param includer the includer to append (may not be null)
         * @return new version of the parse options with different includer
         */
        config_parse_options append_includer(shared_includer includer) const;

        /**
         * Gets the current includer (will be null for the default includer).
         * @return current includer or null
         */
        shared_includer const& get_includer() const;

    private:
        config_parse_options(shared_string origin_desc,
                             bool allow_missing, shared_includer includer,
                             config_syntax syntax = config_syntax::UNSPECIFIED);
        config_parse_options with_fallback_origin_description(shared_string origin_description) const;

        config_syntax _syntax;
        shared_string _origin_description;
        bool _allow_missing;
        shared_includer _includer;
    };
}  // namespace hocon
