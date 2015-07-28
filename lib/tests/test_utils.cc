#include "test_utils.hpp"

using namespace std;

namespace hocon {

    shared_ptr<simple_config_origin> fake_origin(string description, int line_number) {
        return make_shared<simple_config_origin>(move(description), line_number, line_number, origin_type::GENERIC);
    }

    shared_ptr<value> string_token(string text, config_string_type type) {
        return make_shared<value>(unique_ptr<config_string>(
                new config_string(fake_origin(), text, type)));
    }

    shared_ptr<value> bool_token(bool boolean) {
        return make_shared<value>(unique_ptr<config_boolean>(new config_boolean(fake_origin(), boolean)));
    }

    shared_ptr<value> double_token(double number, string original_text) {
        return make_shared<value>(unique_ptr<config_double>(new config_double(fake_origin(), number, original_text)));
    }

    shared_ptr<value> int_token(int number, string original_text) {
        return make_shared<value>(unique_ptr<config_int>(new config_int(fake_origin(), number, original_text)));
    }

    shared_ptr<value> null_token() {
        return make_shared<value>(unique_ptr<config_null>(new config_null(fake_origin())));
    }

    shared_ptr<substitution> substitution_token(shared_ptr<token> inner, bool optional) {
        return make_shared<substitution>(fake_origin(), optional, vector<shared_ptr<token>> { inner });
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

}  // namespace hocon
