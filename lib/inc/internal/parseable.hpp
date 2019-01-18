#pragma once

#include <hocon/config_parseable.hpp>
#include <boost/nowide/fstream.hpp>
#include <internal/simple_config_origin.hpp>
#include <hocon/config_object.hpp>
#include <hocon/config_include_context.hpp>

namespace hocon {

    class config_document;

    class parseable : public config_parseable, public std::enable_shared_from_this<parseable> {
    public:
        static std::shared_ptr<parseable> new_file(std::string input_file_path, config_parse_options options, shared_full_current fpath=nullptr);
        static std::shared_ptr<parseable> new_string(std::string s, config_parse_options options);
        static std::shared_ptr<parseable> new_not_found(std::string what_not_found, std::string message,
                                                        config_parse_options options);

        static config_syntax syntax_from_extension(std::string name);

        void post_construct(config_parse_options const& base_options);
        void post_construct(config_parse_options const& base_options, shared_full_current fpath);

        std::shared_ptr<config_document> parse_config_document();
        shared_object parse(config_parse_options const& options) const override;
        shared_object parse() const;

        shared_value parse_value() const;

        config_parse_options const& options() const override;
        std::shared_ptr<const config_origin> origin() const override;

        virtual std::unique_ptr<std::istream> reader(config_parse_options const& options) const;
        virtual std::unique_ptr<std::istream> reader() const = 0;
        virtual shared_origin create_origin() const = 0;

        virtual config_syntax guess_syntax() const;
        virtual config_syntax content_type() const;
        virtual std::shared_ptr<config_parseable> relative_to(std::string file_name) const;

        std::string to_string() const;

        // Disable copy constructors, as include_context assumes it can hold a reference to parseable.
        parseable() = default;
        parseable(parseable const&) = delete;
        parseable& operator=(parseable const&) = delete;

    private:
        std::shared_ptr<config_document> parse_document(config_parse_options const& base_options) const;
        std::shared_ptr<config_document> parse_document(shared_origin origin,
                                                        config_parse_options const& final_options) const;
        std::shared_ptr<config_document> raw_parse_document(std::unique_ptr<std::istream> stream, shared_origin origin,
                                                            config_parse_options const& options) const;
        std::shared_ptr<config_document> raw_parse_document(shared_origin origin,
                                                            config_parse_options const& options) const;

        shared_value parse_value(config_parse_options const& base_options) const;
        shared_value parse_value(shared_origin origin, config_parse_options const& options) const;
        shared_value raw_parse_value(std::unique_ptr<std::istream> stream,
                                     shared_origin origin,
                                     config_parse_options const& options) const;
        shared_value raw_parse_value(shared_origin origin, config_parse_options const& options) const;

        config_parse_options fixup_options(config_parse_options const& base_options) const;

        std::vector<parseable> _parse_stack;

        shared_origin _initial_origin;
        config_parse_options _initial_options;
        shared_include_context _include_context;

        static const int MAX_INCLUDE_DEPTH;
    };

    class parseable_file : public parseable {
    public:
        parseable_file(std::string input_file_path, config_parse_options options, shared_full_current fpath);
        std::unique_ptr<std::istream> reader() const override;
        shared_origin create_origin() const override;
        config_syntax guess_syntax() const override;

    private:
        std::string _input;
    };

    class parseable_string : public parseable {
    public:
        parseable_string(std::string s, config_parse_options options);
        std::unique_ptr<std::istream> reader() const override;
        shared_origin create_origin() const override;

    private:
        std::string _input;
    };

    // NOTE: this is not a faithful port of the `ParseableResources` class from the
    // upstream, because at least for now we're not going to try to do anything
    // crazy like look for files on the ruby load path.  However, there is a decent
    // chunk of logic elsewhere in the codebase that is written with the assumption
    // that this class will provide the 'last resort' attempt to find a config file
    // before giving up, so we're basically port just enough to have it provide
    // that last resort behavior
    class parseable_resources : public parseable {
    public:
        parseable_resources(std::string resource, config_parse_options options);

        std::unique_ptr<std::istream> reader() const override;
        shared_origin create_origin() const override;

    private:
        std::string _resource;
    };

    // this is a parseable that doesn't exist and just throws when you try to
    // parse it
    class parseable_not_found : public parseable {
    public:
        parseable_not_found(std::string what, std::string message, config_parse_options options);

        std::unique_ptr<std::istream> reader() const override;
        shared_origin create_origin() const override;

    private:
        std::string _what;
        std::string _message;
    };

}  // namespace hocon
