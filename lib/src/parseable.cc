#include <internal/parseable.hpp>
#include <sstream>
#include <boost/algorithm/string/predicate.hpp>
#include <internal/nodes/abstract_config_node.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/simple_config_document.hpp>
#include <internal/exception.hpp>
#include <internal/tokenizer.hpp>
#include <internal/config_document_parser.hpp>
#include <internal/simple_include_context.hpp>

using namespace std;

namespace hocon {

    parseable_file parseable::new_file(std::string input_file_path, shared_parse_options options) {
        return parseable_file(move(input_file_path),  move(options));
    }

    parseable_string parseable::new_string(std::string s, shared_parse_options options) {
        return parseable_string(move(s), move(options));
    }

    parseable_not_found parseable::new_not_found(std::string what_not_found, std::string message,
                                                 shared_parse_options options) {
        return parseable_not_found(move(what_not_found), move(message), move(options));
    }

    void parseable::post_construct(shared_parse_options base_options) {
        _initial_options = fixup_options(base_options);

        // TODO: add include context stuff

        if (_initial_options->get_origin_description()) {
            _initial_origin = make_shared<simple_config_origin>(*_initial_options->get_origin_description());
        } else {
            _initial_origin = create_origin();
        }
    }

    config_syntax parseable::syntax_from_extension(std::string name) {
        if (boost::algorithm::ends_with(name, ".json")) {
            return config_syntax::JSON;
        } else if (boost::algorithm::ends_with(name, ".conf")) {
            return config_syntax::CONF;
        } else {
            return config_syntax::UNSPECIFIED;
        }
    }

    shared_parse_options parseable::options() const {
        return _initial_options;
    }

    shared_ptr<const config_origin> parseable::origin() const {
        return _initial_origin;
    }

    shared_parse_options parseable::fixup_options(shared_parse_options base_options) {
        config_syntax syntax = base_options->get_syntax();
        if (syntax == config_syntax::UNSPECIFIED) {
            syntax = guess_syntax();
        }
        if (syntax == config_syntax::UNSPECIFIED) {
            syntax = config_syntax::CONF;
        }
        config_parse_options modified = base_options->set_syntax(syntax);

        // TODO: add include stuff
        return make_shared<config_parse_options>(modified);
    }

    config_syntax parseable::guess_syntax() {
        return config_syntax::UNSPECIFIED;
    }

    config_syntax parseable::content_type() const {
        return config_syntax::UNSPECIFIED;
    }

    shared_ptr<config_parseable> parseable::relative_to(string file_name) {
        // fall back to classpath; we treat the "filename" as absolute
        // (don't add a package name in front),
        // if it starts with "/" then remove the "/", for consistency
        // with parseable_resrouces.relativeTo
        string resource = file_name;
        if (boost::algorithm::starts_with(file_name, "/")) {
            resource = file_name.substr(1);
        }
        return make_shared<parseable_resources>(resource,
                                                make_shared<config_parse_options>(
                                                        (options()->set_origin_description(nullptr))));
    }

    shared_ptr<config_document> parseable::parse_config_document() {
        return parse_document(_initial_options);
    }

    shared_ptr<config_document> parseable::parse_document(shared_parse_options base_options) {
        // note that we are NOT using our "initialOptions",
        // but using the ones from the passed-in options. The idea is that
        // callers can get our original options and then parse with different
        // ones if they want.
        shared_parse_options options = fixup_options(base_options);

        // passed in options can override origin
        shared_origin origin = _initial_origin;
        if (options->get_origin_description()) {
            origin = make_shared<simple_config_origin>(*options->get_origin_description());
        }
        return parse_document(origin, move(options));
    }

    std::shared_ptr<config_document> parseable::parse_document(shared_origin origin,
                                                               shared_parse_options final_options) {
        try {
            return raw_parse_document(origin, final_options);
        } catch (runtime_error& e) {
            if (final_options->get_allow_missing()) {
                shared_node_list children;
                children.push_back(make_shared<config_node_object>(shared_node_list { }));
                return make_shared<simple_config_document>(make_shared<config_node_root>(children, origin),
                                                           final_options);
            } else {
                throw config_exception("exception loading " + origin->description() + ": " + e.what());
            }
        }
    }

    std::shared_ptr<config_document> parseable::raw_parse_document(shared_origin origin,
                                                                   shared_parse_options options) {
        auto stream = reader(options);

        config_syntax cont_type = content_type();

        shared_parse_options options_with_content_type;
        if (cont_type != config_syntax::UNSPECIFIED) {
            options_with_content_type = make_shared<config_parse_options>(options->set_syntax(cont_type));
        } else {
            options_with_content_type = options;
        }

        return raw_parse_document(move(stream), move(origin), options_with_content_type);
    }

    std::shared_ptr<config_document> parseable::raw_parse_document(std::unique_ptr<std::istream> stream,
                                                                   shared_origin origin,
                                                                   shared_parse_options options) {
        auto tokens = token_iterator(origin, move(stream), options->get_syntax());
        return make_shared<simple_config_document>(config_document_parser::parse(move(tokens), origin, *options), options);
    }



    // TODO: These rely on the ConfigParser, another huge convoluted class to port that has its own ticket
//    const int MAX_INCLUDE_DEPTH = 50;
//    std::shared_ptr<config_object> parseable::parse(shared_parse_options base_options) {
//        if (_parse_stack.size() >= MAX_INCLUDE_DEPTH) {
//            throw config_exception("include statements nested more than " + std::to_string(MAX_INCLUDE_DEPTH) +
//                                           " times, you probably have a cycle in your includes.");
//        }
//
//        _parse_stack.push_back(*this);
//
//    }
//
//    shared_value parseable::raw_parse_value(shared_origin origin, shared_parse_options options) {
//        auto stream = reader(options);
//
//        // after reader() we will have loaded the content type
//        config_syntax cont_type = content_type();
//        shared_parse_options options_with_content_type;
//        if (cont_type != config_syntax::UNSPECIFIED) {
//            options_with_content_type = make_shared<config_parse_options>(options->set_syntax(cont_type));
//        } else {
//            options_with_content_type = options;
//        }
//
//        return raw_parse_value(move(stream), origin, options_with_content_type);
//    }
//
//    shared_value parseable::raw_parse_value(unique_ptr<istream> stream, shared_origin origin,
//                                            shared_parse_options options) {
//        token_iterator tokens(origin, move(stream), options->get_syntax());
//        auto document = config_document_parser::parse(tokens, origin, *options);
//        return config_parser.parse(document, origin, options, include_context());
//    }

    unique_ptr<istream> parseable::reader(shared_parse_options options) {
        return reader();
    }

    /** Parseable file */
    parseable_file::parseable_file(std::string input_file_path, shared_parse_options options) :
        _input(move(input_file_path)) {
        post_construct(options);
    }

    unique_ptr<istream> parseable_file::reader() {
        return unique_ptr<istream>(new boost::nowide::ifstream(_input.c_str()));
    }

    shared_origin parseable_file::create_origin() {
        return make_shared<simple_config_origin>("file: " + _input);
    }

    config_syntax parseable_file::guess_syntax() {
        return syntax_from_extension(_input);
    }

    /** Parseable string */
    parseable_string::parseable_string(std::string s, shared_parse_options options) : _input(move(s)) {
        post_construct(options);
    }

    unique_ptr<istream> parseable_string::reader() {
        return unique_ptr<istringstream>(new istringstream(_input));
    }

    shared_origin parseable_string::create_origin() {
        return make_shared<simple_config_origin>("string");
    }

    /** Parseable resources */
    parseable_resources::parseable_resources(std::string resource, shared_parse_options options) :
            _resource(move(resource)) {
        post_construct(options);
    }

    std::unique_ptr<std::istream> parseable_resources::reader() {
        throw config_exception("reader() should not be called on resources");
    }

    shared_origin parseable_resources::create_origin() {
        return make_shared<simple_config_origin>(_resource);
    }

    /** Parseable Not Found */
    parseable_not_found::parseable_not_found(std::string what, std::string message, shared_parse_options options) :
            _what(move(what)), _message(move(message)) {
        post_construct(options);
    }

    std::unique_ptr<std::istream> parseable_not_found::reader() {
        throw config_exception(_message);
    }

    shared_origin parseable_not_found::create_origin() {
        return make_shared<simple_config_origin>(_what);
    }
}  // namespace hocon
