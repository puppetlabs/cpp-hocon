#include <hocon/parser/config_document_factory.hpp>
#include <internal/parseable.hpp>

using namespace std;

namespace hocon { namespace config_document_factory {

    shared_ptr<config_document> parse_file(string input_file_path, config_parse_options options) {
        return parseable::new_file(move(input_file_path), move(options), nullptr)->parse_config_document();
    }

    shared_ptr<config_document> parse_file(string input_file_path) {
        return parse_file(move(input_file_path), config_parse_options());
    }

    shared_ptr<config_document> parse_string(string s, config_parse_options options) {
        return parseable::new_string(move(s), move(options))->parse_config_document();
    }

    shared_ptr<config_document> parse_string(string s) {
        return parse_string(move(s), config_parse_options());
    }

}}  // namespace hocon
