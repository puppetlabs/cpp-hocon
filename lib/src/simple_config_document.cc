#include <hocon/config_parse_options.hpp>
#include <internal/simple_config_document.hpp>
#include <internal/config_exception.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/tokenizer.hpp>
#include <sstream>

using namespace std;

namespace hocon {
    simple_config_document::simple_config_document(shared_ptr<config_node_root> root, shared_ptr<config_parse_options> opts)
        : _config_node_tree(move(root)), _parse_options(move(opts)) {}

    unique_ptr<config_document> simple_config_document::with_value_text(string path, string new_value) const
    {
        if (new_value.empty()) {
            throw new config_exception("empty value for " + path + " passed to with_value_text");
        }

        shared_origin origin = make_shared<simple_config_origin>("single value parsing");
        token_iterator tokens {origin, unique_ptr<istream>{new stringstream(new_value)}, _parse_options->get_syntax()};

        return unique_ptr<config_document>{new simple_config_document(_config_node_tree, _parse_options)};
    }

    unique_ptr<config_document> simple_config_document::with_value(string path, config_value new_value) const
    {
        return unique_ptr<config_document>{new simple_config_document(_config_node_tree, _parse_options)};
    }

    unique_ptr<config_document> simple_config_document::without_path(string path) const
    {
        return unique_ptr<config_document>{new simple_config_document(_config_node_tree, _parse_options)};
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
