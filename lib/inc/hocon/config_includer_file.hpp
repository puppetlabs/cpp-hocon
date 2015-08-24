#pragma once

namespace hocon {

    /**
     * Implement this <em>in addition to</em> {@link config_includer} if you want to
     * support inclusion of files with the {@code include file("filename")} syntax.
     * If you do not implement this but do implement {@link config_includer},
     * attempts to load files will use the default includer.
     */
    class config_includer_file {
    public:
        /**
         * Parses another item to be included. The returned object typically would
         * not have substitutions resolved. You can throw a config_exception here to
         * abort parsing, or return an empty object, but may not return null.
         *
         * @param context
         *            some info about the include context
         * @param what
         *            the include statement's argument (a file path)
         * @return a non-null config_object
         */
        virtual std::shared_ptr<config_object> include_file(std::shared_ptr<config_include_context> context,
                                                            std::string what) const = 0;
    };

}  // namespace hocon
