#pragma once

#include <hocon/config_parseable.hpp>
#include <boost/nowide/fstream.hpp>
#include <internal/simple_config_origin.hpp>
#include <hocon/config_object.hpp>
#include <hocon/config_include_context.hpp>

namespace hocon {

    class parseable_file;
    class parseable_string;
    class parseable_not_found;

    class parseable : public config_parseable, public std::enable_shared_from_this<parseable> {
    public:
        static parseable_file new_file(std::string input_file_path, shared_parse_options options);
        static parseable_string new_string(std::string s, shared_parse_options options);
        static parseable_not_found new_not_found(std::string what_not_found, std::string message,
                                                 shared_parse_options options);

        static config_syntax syntax_from_extension(std::string name);

        void post_construct(shared_parse_options base_options);

        std::shared_ptr<config_document> parse_config_document();

        shared_parse_options options() const override;
        std::shared_ptr<const config_origin> origin() const override;

        virtual std::unique_ptr<std::istream> reader(shared_parse_options options);
        virtual std::unique_ptr<std::istream> reader() = 0;
        virtual shared_origin create_origin() = 0;
        virtual config_syntax guess_syntax();

        virtual config_syntax content_type() const;
        virtual std::shared_ptr<config_parseable> relative_to(std::string file_name);

    private:
        std::shared_ptr<config_document> parse_document(shared_parse_options base_options);
        std::shared_ptr<config_document> parse_document(shared_origin origin, shared_parse_options final_options);
        std::shared_ptr<config_document> raw_parse_document(std::unique_ptr<std::istream> stream, shared_origin origin,
                                           shared_parse_options options);
        std::shared_ptr<config_document> raw_parse_document(shared_origin origin,
                                                            shared_parse_options options);

        shared_parse_options fixup_options(shared_parse_options base_options);

        std::vector<parseable> _parse_stack;

        shared_origin _initial_origin;
        shared_parse_options _initial_options;
        std::shared_ptr<config_include_context> _include_context;
    };

    class parseable_file : public parseable {
    public:
        parseable_file(std::string input_file_path, shared_parse_options options);
        std::unique_ptr<std::istream> reader() override;
        shared_origin create_origin() override;
        config_syntax guess_syntax() override;

    private:
        std::string _input;
    };

    class parseable_string : public parseable {
    public:
        parseable_string(std::string s, shared_parse_options options);
        std::unique_ptr<std::istream> reader() override;
        shared_origin create_origin() override;

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
        parseable_resources(std::string resource, shared_parse_options options);

        std::unique_ptr<std::istream> reader() override;
        shared_origin create_origin() override;

    private:
        std::string _resource;
    };

    // this is a parseable that doesn't exist and just throws when you try to
    // parse it
    class parseable_not_found : public parseable {
    public:
        parseable_not_found(std::string what, std::string message, shared_parse_options options);

        std::unique_ptr<std::istream> reader() override;
        shared_origin create_origin() override;

    private:
        std::string _what;
        std::string _message;
    };

}  // namespace hocon
