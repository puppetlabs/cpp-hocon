#include <hocon/parser/config_document_factory.hpp>
#include <internal/parseable.hpp>

using namespace std;

namespace hocon { namespace config_document_factory {

    config_document parse_file(string input_file_path, shared_parse_options options) {
        return parseable::new_file(move(input_file_path), move(options)).parse_config_document();
    }

    config_document parse_file(string input_file_path) {
        return parse_file(move(input_file_path), make_shared<config_parse_options>());
    }

    config_document parse_string(string s, shared_parse_options options) {
        return parseable::new_string(move(s), move(options)).parse_config_document();
    }

    config_document parse_string(string s) {
        return parse_string(move(s), make_shared<config_parse_options>());
    }

}}  // namespace hocon
