#pragma once

#include <string>
#include <hocon/config_value.hpp>

namespace hocon {
    /**
     * Represents an individual HOCON or JSON file, preserving all
     * formatting and syntax details.  This can be used to replace
     * individual values and exactly render the original text of the
     * input.
     *
     * <p>
     * Because this object is immutable, it is safe to use from multiple threads and
     * there's no need for "defensive copies."
     *
     * <p>
     * <em>Do not implement interface {@code config_document}</em>; it should only be
     * implemented by the config library. Arbitrary implementations will not work
     * because the library internals assume a specific concrete implementation.
     * Also, this interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class config_document {
    public:
        /**
         * Returns a new config_document that is a copy of the current config_document,
         * but with the desired value set at the desired path. If the path exists, it will
         * remove all duplicates before the final occurrence of the path, and replace the value
         * at the final occurrence of the path. If the path does not exist, it will be added. If
         * the document has an array as the root value, an exception will be thrown.
         *
         * @param path the path at which to set the desired value
         * @param new_value the value to set at the desired path, represented as a string. This
         *                  string will be parsed into a ConfigNode using the same options used to
         *                  parse the entire document, and the text will be inserted
         *                  as-is into the document. Leading and trailing comments, whitespace, or
         *                  newlines are not allowed, and if present an exception will be thrown.
         *                  If a concatenation is passed in for newValue but the document was parsed
         *                  with JSON, the first value in the concatenation will be parsed and inserted
         *                  into the config_document.
         * @return a copy of the config_document with the desired value at the desired path
         */
        virtual config_document with_value_text(std::string path, std::string newValue) const = 0;

        /**
         * Returns a new config_document that is a copy of the current
         * config_document, but with the desired value set at the
         * desired path. Works like {@link #with_value_text(string, string)},
         * but takes a ConfigValue instead of a string.
         *
         * @param path the path at which to set the desired value
         * @param new_value the value to set at the desired path, represented as a ConfigValue.
         *                 The rendered text of the ConfigValue will be inserted into the
         *                 config_document.
         * @return a copy of the config_document with the desired value at the desired path
         */
        virtual config_document with_value(std::string path, config_value new_value) const = 0;

        /**
         * Returns a new config_document that is a copy of the current config_document, but with
         * all values at the desired path removed. If the path does not exist in the document,
         * a copy of the current document will be returned. If there is an array at the root, an exception
         * will be thrown.
         *
         * @param path the path to remove from the document
         * @return a copy of the config_document with the desired value removed from the document.
         */
        virtual config_document without_path(std::string path) const = 0;

        /**
         * Returns a boolean indicating whether or not a config_document has a value at the desired path.
         * null counts as a value for purposes of this check.
         * @param path the path to check
         * @return true if the path exists in the document, otherwise false
         */
        virtual bool has_path(std::string const& path) const = 0;

        /**
         * The original text of the input, modified if necessary with
         * any replaced or added values.
         * @return the modified original text
         */
        virtual std::string render() const = 0;
    };

    /**
     * Config documents compare via rendered strings
     */
    bool operator==(config_document const& lhs, config_document const& rhs);

}  // namespace hocon
