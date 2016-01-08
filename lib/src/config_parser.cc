#include <internal/config_parser.hpp>
#include <internal/nodes/config_node_comment.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/nodes/config_node_complex_value.hpp>

namespace hocon { namespace config_parser {
    using namespace std;

    shared_value parse(shared_ptr<const config_node_root> document,
            config_origin origin,
            shared_parse_options const& options,
            shared_ptr<config_include_context> include_context)
    {
        parse_context context {options->get_syntax(), origin, document, nullptr, include_context};
        return context.parse();
    }

    parse_context::parse_context(config_syntax flavor, config_origin origin, shared_ptr<const config_node_root> document,
            void* includer, shared_ptr<config_include_context> include_context) :
        _line_number(1), _document(document), /*_includer(includer),*/ _include_context(include_context),
        /*_flavor(flavor),*/ _base_origin(origin), array_count(0)
    {}

    shared_value parse_context::parse_value(shared_node_value n, vector<string> comments)
    {
        // TODO
        return {};
    }

    shared_value parse_context::parse()
    {
        shared_value result;
        vector<string> comments;
        bool last_was_newline = false;

        for (auto&& node : _document->children()) {
            if (auto ptr = dynamic_pointer_cast<config_node_comment>(node)) {
                comments.push_back(ptr->comment_text());
                last_was_newline = false;
            } else if (auto ptr = dynamic_pointer_cast<config_node_single_token>(node)) {
                auto t = ptr->get_token();
                if (t->get_token_type() == token_type::NEWLINE) {
                    ++_line_number;
                    if (last_was_newline && !result) {
                        comments.clear();
                    } else if (result) {
                        // TODO
                        // result = result->with_origin(result->origin()->append_comments(move(comments)));
                        break;
                    }
                    last_was_newline = true;
                }
            } else if (auto ptr = dynamic_pointer_cast<config_node_complex_value>(node)) {
                result = parse_value(ptr, move(comments));
                last_was_newline = false;
            }
        }

        return result;
    }
}}  // namespace hocon::config_parser
