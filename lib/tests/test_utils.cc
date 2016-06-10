#include <internal/path_parser.hpp>
#include <hocon/config.hpp>
#include <hocon/config_parse_options.hpp>
#include "test_utils.hpp"
#include "fixtures.hpp"

#include <boost/algorithm/string/replace.hpp>

using namespace std;

namespace hocon { namespace test_utils {

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

    shared_ptr<config_boolean> bool_value(bool b) {
        return make_shared<config_boolean>(fake_origin(), b);
    }

    shared_ptr<config_null> null_value() {
        return make_shared<config_null>(fake_origin());
    }

    shared_ptr<config_string> string_value(string s) {
        return make_shared<config_string>(fake_origin(), s, config_string_type::QUOTED);
    }

    shared_ptr<config_double> double_value(double d) {
        return make_shared<config_double>(fake_origin(), d, "");
    }

    shared_ptr<config_reference> subst(string ref, bool optional) {
        return make_shared<config_reference>(fake_origin(), make_shared<substitution_expression>(path::new_path(ref), optional));
    }

    shared_ptr<config_concatenation> subst_in_string(string ref, bool optional) {
        auto pieces = vector<shared_value> {string_value("start<"), subst(ref, optional), string_value(">end")};
        return make_shared<config_concatenation>(fake_origin(), pieces);
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
        return config::parse_string(move(s), options);
    }

    // note: it's important to put {} or [] at the root if you
    // want to test "invalidity reasons" other than "wrong root"
    static const vector<parse_test> invalid_json_invalid_conf_ = {
        parse_test("{"),
        parse_test("}"),
        parse_test("["),
        parse_test("]"),
        parse_test(","),
        parse_test("10", true), // value not in array or object, lift-json now allows this
        parse_test("\"foo\"", true), // value not in array or object, lift-json allows it
        parse_test("\""), // single quote by itself
        parse_test("[,]", true), // array with just a comma in it; lift is OK with this
        parse_test("[,,]", true), // array with just two commas in it; lift is cool with this too
        parse_test("[1,2,,]", true), // array with two trailing commas
        parse_test("[,1,2]", true), // array with initial comma
        parse_test("{ , }", true), // object with just a comma in it
        parse_test("{ , , }", true), // object with just two commas in it
        parse_test("{ 1,2 }"), // object with single values not key-value pair
        parse_test("{ , \"foo\" : 10 }", true), // object starts with comma
        parse_test("{ \"foo\" : 10 ,, }", true), // object has two trailing commas
        parse_test(" \"a\" : 10 ,, "), // two trailing commas for braceless root object
        parse_test("{ \"foo\" : }"), // no value in object
        parse_test("{ : 10 }"), // no key in object
        parse_test(" \"foo\" : ", true), // no value in object with no braces; lift-json thinks this is acceptable
        parse_test(" : 10 ", true), // no key in object with no braces; lift-json is cool with this too
        parse_test(" \"foo\" : 10 } "), // close brace but no open
        parse_test(" \"foo\" : 10 [ "), // no-braces object with trailing gunk
        parse_test("{ \"foo\" }"), // no value or colon
        parse_test("{ \"a\" : [ }"), // [ is not a valid value
        parse_test("{ \"foo\" : 10, true }"), // non-key after comma
        parse_test("{ foo \n bar : 10 }"), // newline in the middle of the unquoted key
        parse_test("[ 1, \\"), // ends with backslash
        // these two problems are ignored by the lift tokenizer
        parse_test("[:\"foo\", \"bar\"]"), // colon in an array; lift doesn't throw (tokenizer erases it)
        parse_test("[\"foo\" : \"bar\"]"), // colon in an array another way, lift ignores (tokenizer erases it)
        parse_test("[ \"hello ]"), // unterminated string
        parse_test("{ \"foo\" , true }", true), // comma instead of colon, lift is fine with this
        parse_test("{ \"foo\" : true \"bar\" : false }", true), // missing comma between fields, lift fine with this
        parse_test("[ 10, }]"), // array with } as an element
        parse_test("[ 10, {]"), // array with { as an element
        parse_test("{}x"), // trailing invalid token after the root object
        parse_test("[]x"), // trailing invalid token after the root array
        parse_test("{}{}", true), // trailing token after the root object - lift OK with it
        parse_test("{}true", true), // trailing token after the root object; lift ignores the {}
        parse_test("[]{}", true), // trailing valid token after the root array
        parse_test("[]true", true), // trailing valid token after the root array, lift ignores the []
        parse_test("[${]"), // unclosed substitution
        parse_test("[$]"), // '$' by itself
        parse_test("[$  ]"), // '$' by itself with spaces after
        parse_test("[${}]"), // empty substitution (no path)
        parse_test("[${?}]"), // no path with ? substitution
        parse_test("[${ ?foo}]", false, true), // space before ? not allowed
        parse_test(R"({ "a" : [1,2], "b" : y${a}z })"), // trying to interpolate an array in a string
        parse_test(R"({ "a" : { "c" : 2 }, "b" : y${a}z })"), // trying to interpolate an object in a string
        parse_test(R"({ "a" : ${a} })"), // simple cycle
        parse_test(R"([ { "a" : 2, "b" : ${${a}} } ])"), // nested substitution
        parse_test("[ = ]"), // = is not a valid token in unquoted text
        parse_test("[ + ]"),
        parse_test("[ # ]"),
        parse_test("[ ` ]"),
        parse_test("[ ^ ]"),
        parse_test("[ ? ]"),
        parse_test("[ ! ]"),
        parse_test("[ @ ]"),
        parse_test("[ * ]"),
        parse_test("[ & ]"),
        parse_test("[ \\ ]"),
        parse_test("+="),
        parse_test("[ += ]"),
        parse_test("+= 10"),
        parse_test("10 +="),
        parse_test("[ 10e+3e ]"), // "+" not allowed in unquoted strings, and not a valid number
        parse_test("[ \"foo\nbar\" ]", true), // unescaped newline in quoted string, lift doesn't care
        parse_test("[ # comment ]"),
        parse_test("${ #comment }"),
        parse_test("[ // comment ]"),
        parse_test("${ // comment }"),
        parse_test("{ include \"bar\" : 10 }"), // include with a value after it
        parse_test("{ include foo }"), // include with unquoted string
        parse_test("{ include : { \"a\" : 1 } }"), // include used as unquoted key
        parse_test("a="), // no value
        parse_test("a:"), // no value with colon
        parse_test("a= "), // no value with whitespace after
        parse_test("a.b="), // no value with path
        parse_test("{ a= }"), // no value inside braces
        parse_test("{ a: }") // no value with colon inside braces
    };

    // We'll automatically try each of these with whitespace modifications
    // so no need to add every possible whitespace variation
    static const vector<parse_test> valid_json_ = {
        parse_test("{}"),
        parse_test("[]"),
        parse_test(R"({ "foo" : "bar" })"),
        parse_test(R"(["foo", "bar"])"),
        parse_test(R"({ "foo" : 42 })"),
        parse_test("{ \"foo\"\n : 42 }"), // newline after key
        parse_test("{ \"foo\" : \n 42 }"), // newline after colon
        parse_test(R"([10, 11])"),
        parse_test(R"([10,"foo"])"),
        parse_test(R"({ "foo" : "bar", "baz" : "boo" })"),
        parse_test(R"({ "foo" : { "bar" : "baz" }, "baz" : "boo" })"),
        parse_test(R"({ "foo" : { "bar" : "baz", "woo" : "w00t" }, "baz" : "boo" })"),
        parse_test(R"({ "foo" : [10,11,12], "baz" : "boo" })"),
        parse_test(R"([{},{},{},{}])"),
        parse_test(R"([[[[[[]]]]]])"),
        parse_test(R"([[1], [1,2], [1,2,3], []])"), // nested multiple-valued array
        parse_test(R"({"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":42}}}}}}}})"),
        parse_test("[ \"#comment\" ]"), // quoted # comment
        parse_test("[ \"//comment\" ]"), // quoted // comment
        // this long one is mostly to test rendering
        parse_test(R"({ "foo" : { "bar" : "baz", "woo" : "w00t" }, "baz" : { "bar" : "baz", "woo" : [1,2,3,4], "w00t" : true, "a" : false, "b" : 3.14, "c" : null } })"),
        parse_test("{}"),
        parse_test("[ 10e+3 ]", true) // "+" in a number (lift doesn't handle)
    };

    static const vector<parse_test> valid_conf_invalid_json_ = {
        parse_test(""), // empty document
        parse_test(" "), // empty document single space
        parse_test("\n"), // empty document single newline
        parse_test(" \n \n   \n\n\n"), // complicated empty document
        parse_test("# foo"), // just a comment
        parse_test("# bar\n"), // just a comment with a newline
        parse_test("# foo\n//bar"), // comment then another with no newline
        parse_test(R"({ "foo" = 42 })"), // equals rather than colon
        parse_test(R"({ foo { "bar" : 42 } })"), // omit the colon for object value
        parse_test(R"({ foo baz { "bar" : 42 } })"), // omit the colon with unquoted key with spaces
        parse_test(R"( "foo" : 42 )"), // omit braces on root object
        parse_test(R"({ "foo" : bar })"), // no quotes on value
        parse_test(R"({ "foo" : null bar 42 baz true 3.14 "hi" })"), // bunch of values to concat into a string
        parse_test("{ foo : \"bar\" }"), // no quotes on key
        parse_test("{ foo : bar }"), // no quotes on key or value
        parse_test("{ foo.bar : bar }"), // path expression in key
        parse_test("{ foo.\"hello world\".baz : bar }"), // partly-quoted path expression in key
        parse_test("{ foo.bar \n : bar }"), // newline after path expression in key
        parse_test("{ foo  bar : bar }"), // whitespace in the key
        parse_test("{ true : bar }"), // key is a non-string token
        parse_test(R"({ "foo" : "bar", "foo" : "bar2" })", true), // dup keys - lift just returns both
        parse_test("[ 1, 2, 3, ]", true), // single trailing comma (lift fails to throw)
        parse_test("[1,2,3  , ]", true), // single trailing comma with whitespace
        parse_test("[1,2,3\n\n , \n]", true), // single trailing comma with newlines
        parse_test("[1,]", true), // single trailing comma with one-element array
        parse_test("{ \"foo\" : 10, }", true), // extra trailing comma (lift fails to throw)
        parse_test("{ \"a\" : \"b\", }", true), // single trailing comma in object
        parse_test("{ a : b, }"), // single trailing comma in object (unquoted strings)
        parse_test("{ a : b  \n  , \n }"), // single trailing comma in object with newlines
        parse_test("a : b, c : d,"), // single trailing comma in object with no root braces
        parse_test("{ a : b\nc : d }"), // skip comma if there's a newline
        parse_test("a : b\nc : d"), // skip comma if there's a newline and no root braces
        parse_test("a : b\nc : d,"), // skip one comma but still have one at the end
        parse_test("[ foo ]"), // not a known token in JSON
        parse_test("[ t ]"), // start of "true" but ends wrong in JSON
        parse_test("[ tx ]"),
        parse_test("[ tr ]"),
        parse_test("[ trx ]"),
        parse_test("[ tru ]"),
        parse_test("[ trux ]"),
        parse_test("[ truex ]"),
        parse_test("[ 10x ]"), // number token with trailing junk
        parse_test("[ / ]"), // unquoted string "slash"
        parse_test("{ include \"foo\" }"), // valid include
        parse_test("{ include\n\"foo\" }"), // include with just a newline separating from string
        parse_test("{ include\"foo\" }"), // include with no whitespace after it
        parse_test("[ include ]"), // include can be a string value in an array
        parse_test("{ foo : include }"), // include can be a field value also
        parse_test("{ include \"foo\", \"a\" : \"b\" }"), // valid include followed by comma and field
        parse_test("{ foo include : 42 }"), // valid to have a key not starting with include
        parse_test("[ ${foo} ]"),
        parse_test("[ ${?foo} ]"),
        parse_test("[ ${\"foo\"} ]"),
        parse_test("[ ${foo.bar} ]"),
        parse_test("[ abc  xyz  ${foo.bar}  qrs tuv ]"), // value concatenation
        parse_test("[ 1, 2, 3, blah ]"),
        parse_test("[ ${\"foo.bar\"} ]"),
        parse_test("{} # comment"),
        parse_test("{} // comment"),
        parse_test(R"({ "foo" #comment
: 10 })"),
        parse_test(R"({ "foo") // comment
: 10 })"),
        parse_test(R"({ "foo" : #comment
10 })"),
        parse_test(R"({ "foo" : // comment
10 })"),
        parse_test(R"({ "foo" : 10 #comment
})"),
        parse_test(R"({ "foo" : 10 // comment
})"),
        parse_test(R"([ 10, # comment
11])"),
        parse_test(R"([ 10, // comment
11])"),
        parse_test(R"([ 10 # comment
, 11])"),
        parse_test(R"([ 10 // comment
, 11])"),
        parse_test(R"({ /a/b/c : 10 })"), // key has a slash in it
        parse_test("[${ foo.bar}]", false, true), // substitution with leading spaces
        parse_test("[${foo.bar }]", false, true), // substitution with trailing spaces
        parse_test("[${ \"foo.bar\"}]", false, true), // substitution with leading spaces and quoted
        parse_test("[${\"foo.bar\" }]", false, true), // substitution with trailing spaces and quoted
        parse_test(R"([ ${"foo""bar"} ])"), // multiple strings in substitution
        parse_test(R"([ ${foo  "bar"  baz} ])"), // multiple strings and whitespace in substitution
        parse_test("[${true}]"), // substitution with unquoted true token
        parse_test("a = [], a += b"), // += operator with previous init
        parse_test("{ a = [], a += 10 }"), // += in braces object with previous init
        parse_test("a += b"), // += operator without previous init
        parse_test("{ a += 10 }"), // += in braces object without previous init
        parse_test("[ 10e3e3 ]"), // two exponents. this should parse to a number plus string "e3"
        parse_test("[ 1-e3 ]"), // malformed number should end up as a string instead
        parse_test("[ 1.0.0 ]"), // two decimals, should end up as a string
        parse_test("[ 1.0. ]") // trailing decimal should end up as a string
    };

    template<typename T>
    vector<T> vector_join(vector<T> const& a, vector<T> const& b) {
        vector<T> result = a;
        result.insert(result.end(), b.begin(), b.end());
        return result;
    }

    vector<parse_test> const& invalid_json() {
        static const auto result = vector_join(valid_conf_invalid_json_, invalid_json_invalid_conf_);
        return result;
    }

    vector<parse_test> const& invalid_conf() {
        return invalid_json_invalid_conf_;
    }

    vector<parse_test> const& valid_json() {
        return valid_json_;
    }

    // .conf is a superset of JSON so valid_json just goes in here
    vector<parse_test> const& valid_conf() {
        static const auto result = vector_join(valid_conf_invalid_json_, valid_json_);
        return result;
    }

    std::vector<parse_test> whitespace_variations(vector<parse_test> const& tests, bool valid_in_lift) {
        vector<function<string(string const&)>> variations = {
            [](string const& s) { return s; },
            [](string const& s) { return " " + s; },
            [](string const& s) { return s + " "; },
            [](string const& s) { return " " + s + " "; },
            [](string const& s) { return boost::algorithm::replace_all_copy(s, " ", ""); }, // this would break with whitespace in a key or value
            [](string const& s) { return boost::algorithm::replace_all_copy(s, ":", " : "); }, // could break with : in a key or value
            [](string const& s) { return boost::algorithm::replace_all_copy(s, ",", " , "); } // could break with , in a key or value
        };

        vector<parse_test> new_tests;
        for (auto const& t : tests) {
            if (t.whitespace_matters) {
                new_tests.push_back(t);
            } else {
                // TODO: withNonAscii, requires finishing support for unicode spaces
                for (auto v : variations) {
                    new_tests.emplace_back(v(t.test), t.lift_behavior_unexpected);
                }
            }
        }
        return new_tests;
    }

    std::string fixture_path(std::string fixture_name) {
        return string(TEST_FILE_DIR) + "/fixtures/" + fixture_name;
    }
}}  // namespace hocon::test_utils
