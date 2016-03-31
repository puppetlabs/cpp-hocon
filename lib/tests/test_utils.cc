#include <internal/path_parser.hpp>
#include <hocon/config.hpp>
#include <hocon/config_parse_options.hpp>
#include "test_utils.hpp"

using namespace std;

namespace hocon {

    shared_origin fake_origin(string description, int line_number) {
        return make_shared<simple_config_origin>(move(description), line_number, line_number, origin_type::GENERIC);
    }

    /** Tokens */
    shared_ptr<value> string_token(string text, config_string_type type) {
        return make_shared<value>(make_shared<config_string>(fake_origin(), text, type));
    }

    shared_ptr<value> bool_token(bool boolean) {
        return make_shared<value>(make_shared<config_boolean>(fake_origin(), boolean));
    }

    shared_ptr<value> long_token(int64_t number, string original_text) {
        return make_shared<value>(make_shared<config_long>(fake_origin(), number, original_text));
    }

    shared_ptr<value> double_token(double number, string original_text) {
        return make_shared<value>(make_shared<config_double>(fake_origin(), number, original_text));
    }

    shared_ptr<value> int_token(int number, string original_text) {
        return make_shared<value>(make_shared<config_int>(fake_origin(), number, original_text));
    }

    shared_ptr<value> null_token() {
        return make_shared<value>(make_shared<config_null>(fake_origin()));
    }

    shared_ptr<substitution> substitution_token(shared_token inner, bool optional) {
        return make_shared<substitution>(fake_origin(), optional, token_list { inner });
    }

    shared_ptr<line> line_token(int line_number) {
        return make_shared<line>(fake_origin("fake", line_number));
    }

    shared_ptr<ignored_whitespace> whitespace_token(string whitespace) {
        return make_shared<ignored_whitespace>(fake_origin(), whitespace);
    }

    shared_ptr<unquoted_text> unquoted_text_token(string text) {
        return make_shared<unquoted_text>(fake_origin(), text);
    }

    shared_ptr<double_slash_comment> double_slash_comment_token(string text) {
        return make_shared<double_slash_comment>(fake_origin(), text);
    }

    shared_ptr<hash_comment> hash_comment_token(string text) {
        return make_shared<hash_comment>(fake_origin(), text);
    }

    /** Nodes */
    shared_ptr<config_node_single_token> colon_node() {
        return make_shared<config_node_single_token>(tokens::colon_token());
    }

    shared_ptr<config_node_single_token> open_brace_node() {
        return make_shared<config_node_single_token>(tokens::open_curly_token());
    }

    shared_ptr<config_node_single_token> close_brace_node() {
        return make_shared<config_node_single_token>(tokens::close_curly_token());
    }

    shared_ptr<config_node_single_token> space_node() {
        return make_shared<config_node_single_token>(unquoted_text_token(" "));
    }

    shared_ptr<config_node_single_token> comma_node() {
        return make_shared<config_node_single_token>(tokens::comma_token());
    }

    shared_ptr<config_node_field> node_key_value_pair(shared_ptr<config_node_path> key, shared_node_value value) {
        shared_node_list nodes { key, space_node(), colon_node(), space_node(), value };
        return make_shared<config_node_field>(nodes);
    }

    shared_ptr<config_node_path> node_key(string key) {
        return make_shared<config_node_path>(path_parser::parse_path_node(key));
    }

    shared_ptr<config_node_simple_value> int_node(int number) {
        return make_shared<config_node_simple_value>(int_token(number, to_string(number)));
    }

    shared_ptr<config_node_simple_value> long_node(int64_t number) {
        return make_shared<config_node_simple_value>(long_token(number, to_string(number)));
    }

    shared_ptr<config_node_simple_value> double_node(double number) {
        return make_shared<config_node_simple_value>(double_token(number, to_string(number)));
    }

    shared_ptr<config_node_simple_value> bool_node(bool value) {
        return make_shared<config_node_simple_value>(bool_token(value));
    }

    shared_ptr<config_node_simple_value> string_node(string text) {
        return make_shared<config_node_simple_value>(string_token(text));
    }

    shared_ptr<config_node_simple_value> null_node() {
        return make_shared<config_node_simple_value>(null_token());
    }

    shared_ptr<config_node_simple_value> unquoted_text_node(string text) {
        return make_shared<config_node_simple_value>(unquoted_text_token(text));
    }

    shared_ptr<config_node_simple_value> substitution_node(shared_token key, bool optional) {
        return make_shared<config_node_simple_value>(substitution_token(key, optional));
    }

    shared_ptr<config_node_single_token> line_node(int line_number) {
        return make_shared<config_node_single_token>(line_token(line_number));
    }

    shared_ptr<config_node_single_token> whitespace_node(string whitespace) {
        return make_shared<config_node_single_token>(whitespace_token(whitespace));
    }

    shared_ptr<config_node_comment> double_slash_comment_node(string text) {
        return make_shared<config_node_comment>(double_slash_comment_token(text));
    }

    shared_ptr<config_int> int_value(int i) {
        return make_shared<config_int>(fake_origin(), i, "");
    }

    /** Paths */
    path test_path(initializer_list<string> path_strings) {
        return path(vector<string> { path_strings });
    }

    shared_object parse_object(std::string s) {
        return parse_config(move(s))->root();
    }

    shared_config parse_config(std::string s) {
        auto options = config_parse_options().
            set_origin_description(make_shared<string>("test string")).set_syntax(config_syntax::CONF);
        return config::parse_string(move(s), make_shared<config_parse_options>(move(options)));
    }

}  // namespace hocon
