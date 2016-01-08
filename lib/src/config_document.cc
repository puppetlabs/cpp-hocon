#include <hocon/config_parse_options.hpp>
#include <hocon/parser/config_document.hpp>
#include <internal/config_exception.hpp>
#include <internal/tokenizer.hpp>
#include <internal/config_document_parser.hpp>
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace hocon {
    config_document::config_document(shared_ptr<const config_node_root> root,
                                     shared_parse_options opts)
        : _config_node_tree(move(root)), _parse_options(move(opts)) {}

    config_document config_document::with_value_text(string path, string new_value) const
    {
        if (new_value.empty()) {
            throw new config_exception("empty value for " + path + " passed to with_value_text");
        }

        auto origin = config_origin("single value parsing");
        token_iterator tokens {origin, unique_ptr<istream>{new stringstream(new_value)}, _parse_options->get_syntax()};
        shared_node_value parsed_value = config_document_parser::parse_value(move(tokens), origin, *_parse_options);

        return config_document(
                _config_node_tree->set_value(path, parsed_value, _parse_options->get_syntax()),
                _parse_options);
    }

    config_document config_document::with_value(string path,
                                                            shared_ptr<config_value> new_value) const
    {
        if (!new_value) {
            throw config_exception("null value for " + path + " passed to with_value");
        }
        config_render_options options = config_render_options();
        options = options.set_origin_comments(false);
        string rendered = new_value->render(options);
        boost::trim(rendered);
        return with_value_text(path, rendered);
    }

    config_document config_document::without_path(string path) const
    {
        return config_document(
                _config_node_tree->set_value(path, nullptr, _parse_options->get_syntax()), _parse_options);
    }

    bool config_document::has_path(string const& path) const
    {
        return _config_node_tree->has_value(path);
    }

    string config_document::render() const
    {
        return _config_node_tree->render();
    }

    bool operator==(config_document const& lhs, config_document const& rhs)
    {
        return lhs.render() == rhs.render();
    }

}  // namespace hocon
