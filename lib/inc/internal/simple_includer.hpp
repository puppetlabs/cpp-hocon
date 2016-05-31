#pragma once

#include <hocon/config_includer.hpp>
#include <hocon/config_includer_file.hpp>
#include <hocon/config_parse_options.hpp>
#include <internal/full_includer.hpp>

namespace hocon {

    class name_source;

    class simple_includer : public config_includer, public config_includer_file, public std::enable_shared_from_this<simple_includer> {
    public:
        simple_includer(shared_includer fallback);

        shared_includer with_fallback(shared_includer fallback) const override;

        shared_object include(shared_include_context context, std::string what) const override;

        shared_object include_without_fallback(shared_include_context context, std::string what) const;

        shared_object include_file(shared_include_context context, std::string what) const override;

        static shared_object include_file_without_fallback(shared_include_context context, std::string what);

        static config_parse_options clear_for_include(config_parse_options const& options);

        static shared_object from_basename(std::shared_ptr<name_source> source,
                                           std::string name,
                                           config_parse_options options);

        static std::shared_ptr<const full_includer> make_full(std::shared_ptr<const config_includer> includer);

    private:
        shared_includer _fallback;

        // the Proxy is a proxy for an application-provided includer that uses our
        // default implementations when the application-provided includer doesn't
        // have an implementation.
        class proxy : public full_includer, public std::enable_shared_from_this<proxy> {
        public:
            proxy(std::shared_ptr<const config_includer> delegate);

            shared_includer with_fallback(shared_includer fallback) const;

            shared_object include(shared_include_context context, std::string what) const;

            shared_object include_file(shared_include_context context, std::string what) const;

            static std::shared_ptr<const full_includer> make_full(shared_includer includer);

        private:
            std::shared_ptr<const config_includer> _delegate;
        };
    };

    class name_source {
    public:
        virtual shared_parseable name_to_parseable(std::string name,
                                                   config_parse_options parse_options) const = 0;
    };

    class relative_name_source : public name_source {
    public:
        relative_name_source(shared_include_context context);

        shared_parseable name_to_parseable(std::string name,
                                           config_parse_options parse_options) const override;
    private:
        const shared_include_context _context;
    };

    class file_name_source : public name_source {
    public:
        shared_parseable name_to_parseable(std::string name,
                                           config_parse_options parse_options) const override;
    };

}  // namespace hocon
