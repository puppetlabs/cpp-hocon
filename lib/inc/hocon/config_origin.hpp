#pragma once

#include "types.hpp"
#include <memory>
#include <string>
#include <vector>
#include "export.h"

namespace hocon {

    /**
     * Represents the origin (such as filename and line number) of a
     * {@link config_value} for use in error messages. Obtain the origin of a value
     * with {@link config_value#origin}. Exceptions may have an origin, see
     * {@link config_exception#origin}, but be careful because
     * <code>config_exception.origin()</code> may return null.
     *
     * <p>
     * It's best to use this interface only for debugging; its accuracy is
     * "best effort" rather than guaranteed, and a potentially-noticeable amount of
     * memory could probably be saved if origins were not kept around, so in the
     * future there might be some option to discard origins.
     *
     * <p>
     * <em>Do not implement this interface</em>; it should only be implemented by
     * the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class config_origin {
    public:
        /**
         * Returns a string describing the origin of a value or exception. This will
         * never return null.
         *
         * @return string describing the origin
         */
        LIBCPP_HOCON_EXPORT virtual std::string const& description() const = 0;

        /**
         * Returns a {@code ConfigOrigin} based on this one, but with the given
         * line number. This origin must be a FILE, URL or RESOURCE. Does not modify
         * this instance or any {@code ConfigValue}s with this origin (since they are
         * immutable).  To set the returned origin to a  {@code ConfigValue}, use
         * {@link ConfigValue#withOrigin}.
         *
         * <p>
         * Note that when the given lineNumber are equal to the lineNumber on this
         * object, a new instance may not be created and {@code this} is returned
         * directly.
         *
         * @since 1.3.0
         *
         * @param lineNumber the new line number
         * @return the created ConfigOrigin
         */
        LIBCPP_HOCON_EXPORT virtual shared_origin with_line_number(int line_number) const = 0;

        /**
         * Returns a line number where the value or exception originated. This will
         * return -1 if there's no meaningful line number.
         *
         * @return line number or -1 if none is available
         */
        LIBCPP_HOCON_EXPORT virtual int line_number() const = 0;

        /**
         * Returns any comments that appeared to "go with" this place in the file.
         * Often an empty list, but never null. The details of this are subject to
         * change, but at the moment comments that are immediately before an array
         * element or object field, with no blank line after the comment, "go with"
         * that element or field.
         *
         * @return any comments that seemed to "go with" this origin, empty list if
         *         none
         */
        LIBCPP_HOCON_EXPORT virtual std::vector<std::string> const& comments() const = 0;

        /**
         * Returns a {@code config_origin} based on this one, but with the given
         * comments. Does not modify this instance or any {@code config_value}s with
         * this origin (since they are immutable).  To set the returned origin to a
         * {@code config_value}, use {@link config_value#with_origin}.
         *
         * <p>
         * Note that when the given comments are equal to the comments on this object,
         * a new instance may not be created and {@code this} is returned directly.
         *
         * @since 1.3.0
         *
         * @param comments the comments used on the returned origin
         * @return the config_origin with the given comments
         */
        LIBCPP_HOCON_EXPORT virtual shared_origin with_comments(std::vector<std::string> comments) const = 0;
    };

}  // namespace hocon
