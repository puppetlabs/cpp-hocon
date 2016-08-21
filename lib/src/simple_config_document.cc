#include <hocon/config_parse_options.hpp>
#include <internal/simple_config_document.hpp>
#include <hocon/config_exception.hpp>
#include <internal/tokenizer.hpp>
#include <internal/config_document_parser.hpp>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {
    simple_config_document::simple_config_document(shared_ptr<const config_node_root> root,
                                                   config_parse_options opts)
        : _config_node_tree(move(root)), _parse_options(move(opts)) {}

    unique_ptr<config_document> simple_config_document::with_value_text(string path, string new_value) const
    {
        if (new_value.empty()) {
            throw new config_exception(_("empty value for {1} passed to with_value_text", path));
        }

        shared_origin origin = make_shared<simple_config_origin>("single value parsing");
        token_iterator tokens {origin, unique_ptr<istream>{new stringstream(new_value)}, _parse_options.get_syntax()};
        shared_node_value parsed_value = config_document_parser::parse_value(move(tokens), origin, _parse_options);

        return unique_ptr<config_document>{new simple_config_document(
                _config_node_tree->set_value(path, parsed_value, _parse_options.get_syntax()),
                _parse_options)};
    }

    unique_ptr<config_document> simple_config_document::with_value(string path,
                                                                   shared_ptr<config_value> new_value) const
    {
        if (!new_value) {
            throw config_exception(_("null value for {1} passed to with_value", path));
        }
        config_render_options options = config_render_options();
        options = options.set_origin_comments(false);
        string rendered = new_value->render(options);
        boost::trim(rendered);
        return with_value_text(path, rendered);
    }

    unique_ptr<config_document> simple_config_document::without_path(string path) const
    {
        return unique_ptr<config_document>{new simple_config_document(
                _config_node_tree->set_value(path, nullptr, _parse_options.get_syntax()), _parse_options)};
    }

    bool simple_config_document::has_path(string const& path) const
    {
        return _config_node_tree->has_value(path);
    }

    string simple_config_document::render() const
    {
        return _config_node_tree->render();
    }

    bool operator==(config_document const& lhs, config_document const& rhs)
    {
        return lhs.render() == rhs.render();
    }

}  // namespace hocon
