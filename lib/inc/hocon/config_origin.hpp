#pragma once

#include "types.hpp"
#include <memory>
#include <vector>
#include <string>
#include "export.h"

namespace hocon {

    enum class origin_type { GENERIC, FILE, RESOURCE };

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
        //-------------------- PUBLIC API --------------------
        /**
         * Returns a string describing the origin of a value or exception. This will
         * never return null.
         *
         * @return string describing the origin
         */
        LIBCPP_HOCON_EXPORT std::string description() const;

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
        LIBCPP_HOCON_EXPORT config_origin with_line_number(int line_number) const;

        /**
         * Returns a line number where the value or exception originated. This will
         * return -1 if there's no meaningful line number.
         *
         * @return line number or -1 if none is available
         */
        LIBCPP_HOCON_EXPORT int line_number() const;

        //-------------------- INTERNAL API --------------------
        // Declares a null origin.
        config_origin() {}
        config_origin(std::string description, int line_number = -1, int end_line_number = -1,
            origin_type org_type = origin_type::GENERIC, std::string resource_or_null = {},
            std::vector<std::string> comments_or_null = {});

        config_origin append_comments(std::vector<std::string> comments) const;

        operator bool() const { return _impl != nullptr; }

        bool operator==(const config_origin &other) const;
        bool operator!=(const config_origin &other) const;

    private:
        // Use a shared_ptr implementation to share identical origins between multiple objects.
        struct simple_config_origin;
        std::shared_ptr<const simple_config_origin> _impl;
    };

}  // namespace hocon
