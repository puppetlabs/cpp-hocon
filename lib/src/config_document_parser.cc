#include <internal/config_document_parser.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/nodes/config_node_comment.hpp>
#include <internal/nodes/config_node_concatenation.hpp>
#include <internal/nodes/config_node_simple_value.hpp>
#include <internal/path_parser.hpp>
#include <internal/config_util.hpp>

#include <unordered_map>
#include <internal/nodes/config_node_field.hpp>
#include <internal/nodes/config_node_array.hpp>

using namespace std;

namespace hocon { namespace config_document_parser {

    shared_ptr<config_node_root> parse(token_iterator tokens, config_origin origin,
                                                                    config_parse_options options)
    {
        parse_context context { options.get_syntax(), move(origin), move(tokens) };
        return context.parse();
    }

    shared_node_value parse_value(token_iterator tokens, config_origin origin,
                                                          config_parse_options options)
    {
        parse_context context { options.get_syntax(), move(origin), move(tokens) };
        return context.parse_single_value();
    }

    /** Parse context */
    parse_context::parse_context(config_syntax flavor, config_origin origin, token_iterator tokens) :
        _line_number(1), _tokens(move(tokens)), _flavor(flavor), _base_origin(move(origin)) { }

    shared_token parse_context::pop_token() {
        if (_buffer.empty()) {
            return _tokens.next();
        }
        shared_token top = _buffer.top();
        _buffer.pop();
        return top;
    }

    shared_token parse_context::next_token() {
        shared_token t = pop_token();
        if (_flavor == config_syntax::JSON) {
            if (t->get_token_type() == token_type::UNQUOTED_TEXT && !is_unquoted_whitespace(t)) {
                throw parse_exception("Token not allowed in valid JSON: '" + t->token_text() + "'");
            } else if (t->get_token_type() == token_type::SUBSTITUTION) {
                throw parse_exception("Substitutions (${} syntax) not allowed in JSON");
            }
        }
        return t;
    }

    shared_token parse_context::next_token_collecting_whitespace(shared_node_list& nodes) {
        while (true) {
            shared_token t = next_token();
            if (t->get_token_type() == token_type::IGNORED_WHITESPACE || t->get_token_type() == token_type::NEWLINE
                    || is_unquoted_whitespace(t)) {
                nodes.push_back(make_shared<config_node_single_token>(t));
                if (t->get_token_type() == token_type::NEWLINE) {
                    _line_number = t->line_number() + 1;
                }
            } else if (t->get_token_type() == token_type::COMMENT) {
                nodes.push_back(make_shared<config_node_comment>(t));
            } else {
                if (t->line_number() >= 0) {
                    _line_number = t->line_number();
                }
                return t;
            }
        }
    }

    void parse_context::put_back(shared_token token) {
        _buffer.push(token);
    }

    bool parse_context::check_element_separator(shared_node_list& nodes) {
        if (_flavor == config_syntax::JSON) {
            shared_token t = next_token_collecting_whitespace(nodes);
            if (t->get_token_type() == token_type::COMMA) {
                nodes.push_back(make_shared<config_node_single_token>(t));
                return true;
            } else {
                put_back(t);
                return false;
            }
        } else {
            bool saw_newline = false;
            shared_token t = next_token();
            while (true) {
                if (t->get_token_type() == token_type::IGNORED_WHITESPACE || is_unquoted_whitespace(t)) {
                    nodes.push_back(make_shared<config_node_single_token>(t));
                } else if (t->get_token_type() == token_type::COMMENT) {
                    nodes.push_back(make_shared<config_node_comment>(t));
                } else if (t->get_token_type() == token_type::NEWLINE) {
                    saw_newline = true;
                    _line_number++;
                    nodes.push_back(make_shared<config_node_single_token>(t));
                    // we want to continue to also eat a comma if there is one
                } else if (t->get_token_type() == token_type::COMMA) {
                    nodes.push_back(make_shared<config_node_single_token>(t));
                    return true;
                } else {
                    // non-newline-or-comma
                    put_back(t);
                    return saw_newline;
                }
                t = next_token();
            }
        }
    }

    shared_node_value parse_context::consolidate_values(shared_node_list &nodes) {
        // this trick is not done in JSON
        if (_flavor == config_syntax::JSON) {
            return nullptr;
        }

        // create only if we have value tokens
        shared_node_list values;
        int value_count = 0;

        // ignore a newline up front
        shared_token t = next_token_collecting_whitespace(nodes);
        while (t) {
            shared_node_value v = nullptr;
            if (t->get_token_type() == token_type::IGNORED_WHITESPACE) {
                values.push_back(make_shared<config_node_single_token>(t));
                t = next_token();
                continue;
            } else if (t->get_token_type() == token_type::VALUE || t->get_token_type() == token_type::UNQUOTED_TEXT ||
                    t->get_token_type() == token_type::SUBSTITUTION || t->get_token_type() == token_type::OPEN_CURLY ||
                    t->get_token_type() == token_type::OPEN_SQUARE) {
                // there may be newlines within objects and arrays
                v = parse_value(t);
                value_count++;
            } else {
                break;
            }
            if (v == nullptr) {
                throw config_exception("no value");
            }

            values.push_back(v);
            t = next_token();  // but don't consolidate across a newline
        }

        // No concatenation was seen, but a single value may have been parsed, so return it, and put back
        // all succeeding tokens
        put_back(t);
        if (value_count < 2) {
            shared_node_value value = nullptr;
            for (auto&& node : values) {
                if (auto found_value = dynamic_pointer_cast<abstract_config_node_value>(node)) {
                    value = found_value;
                } else if (!value) {
                    nodes.push_back(node);
                } else {
                    put_back(node->get_tokens()[0]);
                }
            }
            return value;
        }

        // Put back any trailing whitespace, as the parent object is responsible for tracking
        // any leading/trailing whitespace
        for (int i = static_cast<int>(values.size() - 1); i >= 0; i--) {
            if (auto single_token = dynamic_pointer_cast<config_node_single_token>(values[i])) {
                put_back(single_token->get_token());
                values.erase(values.begin() + i);
            } else {
                break;
            }
        }
        return make_shared<config_node_concatenation>(values);
    }

    string parse_context::add_quote_suggestion(std::string bad_token, std::string message) {
        return add_quote_suggestion(bad_token, message, _equals_count > 0, nullptr);
    }

    string parse_context::add_quote_suggestion(string bad_token, string message, bool inside_equals,
                                                    path *last_path)
    {
        string prev_field_name = last_path == nullptr ? "" : last_path->render();

        string part;
        if (bad_token == tokens::end_token()->to_string()) {
            // EOF require special handling for the error to make sense
            if (!prev_field_name.empty()) {
                part = message + " (if you intended '" + prev_field_name
                       + "' to be part of a value, instead of a key, try adding double quotes around the whole value";
            } else {
                return message;
            }
        } else {
            if (!prev_field_name.empty()) {
                part = message + " (if you intended " + bad_token
                       + " to be part of the value for '" + prev_field_name +
                        "', try enclosing the value in double quotes";
            } else {
                part = message + " (if you intended " + bad_token
                       + " to be part of a key or string value, try enclosing the key or value in double quotes";
            }
        }

        if (inside_equals) {
            return part + ", or you may be able to rename the file .properties rather than .conf)";
        } else {
            return part + ")";
        }
    }

    shared_node_value parse_context::parse_value(shared_token t) {
        shared_node_value v = nullptr;
        int starting_equals_count = _equals_count;

        if (t->get_token_type() == token_type::VALUE || t->get_token_type() == token_type::UNQUOTED_TEXT ||
                t->get_token_type() == token_type::SUBSTITUTION) {
            v = make_shared<config_node_simple_value>(t);
        } else if (t->get_token_type() == token_type::OPEN_CURLY) {
            v = parse_object(true);
        } else if (t->get_token_type() == token_type::OPEN_SQUARE) {
            v = parse_array();
        } else {
            throw parse_exception(add_quote_suggestion(t->to_string(),
                                                   "Expecting a value but got wrong token: " + t->to_string()));
        }

        if (_equals_count != starting_equals_count) {
            throw config_exception("Bug in config parser: unbalanced quals count");
        }

        return v;
    }

    shared_ptr<config_node_path> parse_context::parse_key(shared_token token) {
        if (_flavor == config_syntax::JSON) {
            if (tokens::is_value_with_type(token, config_value_type::STRING)) {
                single_token_iterator it(token);
                return make_shared<config_node_path>(path_parser::parse_path_node_expression(it, config_origin()));
            } else {
                throw parse_exception("Expecting close brace } or a field name here, got " + token->to_string());
            }
        } else {
            token_list expression;
            shared_token t = token;
            while (t->get_token_type() == token_type::VALUE || t->get_token_type() == token_type::UNQUOTED_TEXT) {
                expression.push_back(t);
                t = next_token();  // note - don't cross a newline
            }

            if (expression.empty()) {
                throw parse_exception("expecting a close brace or a field name here, got " + t->to_string());
            }

            put_back(t);
            token_list_iterator it { expression };
            return make_shared<config_node_path>(path_parser::parse_path_node_expression(it, config_origin()));
        }
    }

    bool parse_context::is_include_keyword(shared_token t) {
        return t->get_token_type() == token_type::UNQUOTED_TEXT && t->token_text() == "include";
    }

    bool parse_context::is_unquoted_whitespace(shared_token t) {
        if (t->get_token_type() != token_type::UNQUOTED_TEXT) {
            return false;
        }

        string s = t->token_text();
        for (char c : s) {
            if (!is_whitespace(c)) {
                return false;
            }
        }
        return true;
    }

    bool parse_context::is_key_value_separator(shared_token t) {
        if (_flavor == config_syntax::JSON) {
            return t->get_token_type() == token_type::COLON;
        } else {
            return t->get_token_type() == token_type::COLON || t->get_token_type() == token_type::EQUALS ||
                                                               t->get_token_type() == token_type::PLUS_EQUALS;
        }
    }

    shared_ptr<config_node_include> parse_context::parse_include(shared_node_list& children) {
        shared_token t = next_token_collecting_whitespace(children);

        // we either have a quoted string or the "file()" syntax
        if (t->get_token_type() == token_type::UNQUOTED_TEXT) {
            string kind_text = t->token_text();

            config_include_kind kind;
            if (kind_text == "url(") {
                kind = config_include_kind::URL;
            } else if (kind_text == "file(") {
                kind = config_include_kind::FILE;
            } else if (kind_text == "classpath(") {
                kind = config_include_kind::CLASSPATH;
            } else {
                throw parse_exception("expecting include parameter to be quoted filename, file(), classpath(), or "
                                              "url(). No spaces are allowed before the open paren. Not expecting: " +
                                                t->to_string());
            }

            children.push_back(make_shared<config_node_single_token>(t));

            // skip space inside parens
            t = next_token_collecting_whitespace(children);

            // quoted string
            if (!tokens::is_value_with_type(t, config_value_type::STRING)) {
                throw parse_exception("expecting a quoted string inside file(), classpath(), or url(), rather than" +
                t->to_string());
            }
            children.push_back(make_shared<config_node_simple_value>(t));

            // skip space inside parens
            t = next_token_collecting_whitespace(children);

            if (t->token_text() != ")") {
                throw parse_exception("expecting a close parentheses ')' here, not: " + t->to_string());
            }
            return make_shared<config_node_include>(children, kind);
        } else if (tokens::is_value_with_type(t, config_value_type::STRING)) {
            children.push_back(make_shared<config_node_simple_value>(t));
            return make_shared<config_node_include>(children, config_include_kind::HEURISTIC);
        } else {
            throw parse_exception("include keyword is not followed by a quoted string, but by: " + t->to_string());
        }
    }

    shared_ptr<config_node_complex_value> parse_context::parse_object(bool had_open_curly) {
        // invoked just after the OPEN_CURLY (or START, if !hadOpenCurly)
        bool after_comma = false;
        path* last_path = nullptr;
        bool last_inside_equals = false;
        shared_node_list object_nodes;
        shared_node_list key_value_nodes;
        unordered_map<string, bool> keys;

        if (had_open_curly) {
            object_nodes.push_back(make_shared<config_node_single_token>(tokens::open_curly_token()));
        }

        while (true) {
            shared_token t = next_token_collecting_whitespace(object_nodes);
            if (t->get_token_type() == token_type::CLOSE_CURLY) {
                if (_flavor == config_syntax::JSON && after_comma) {
                    throw parse_exception(add_quote_suggestion(t->to_string(),
                        "expecting a field name after a comma, got a close brace '}' instead"));
                } else if (!had_open_curly) {
                    throw parse_exception(add_quote_suggestion(t->to_string(),
                           "unbalanced close brace '}' with no open brace"));
                }
                object_nodes.push_back(make_shared<config_node_single_token>(tokens::close_curly_token()));
                break;
            } else if (t->get_token_type() == token_type::END && !had_open_curly) {
                put_back(t);
                break;
            } else if (_flavor != config_syntax::JSON && is_include_keyword(t)) {
                shared_node_list include_nodes;
                include_nodes.push_back(make_shared<config_node_single_token>(t));
                shared_ptr<config_node_include> inc = parse_include(include_nodes);
                object_nodes.push_back(inc);
                after_comma = false;
            } else {
                key_value_nodes.clear();
                shared_token key_token = t;
                auto key_path = parse_key(key_token);
                key_value_nodes.push_back(key_path);
                shared_token after_key = next_token_collecting_whitespace(key_value_nodes);
                bool inside_equals = false;

                shared_node_value next_value = nullptr;
                if (_flavor == config_syntax::CONF && after_key->get_token_type() == token_type::OPEN_CURLY) {
                    // can omit the ':' or '=' befor an object value
                    next_value = parse_value(after_key);
                } else {
                    if (!is_key_value_separator(after_key)) {
                        throw parse_exception(add_quote_suggestion(after_key->to_string(),
                        "Key '" + key_path->render() + "' may not be followed by token: " + after_key->to_string()));
                    }

                    key_value_nodes.push_back(make_shared<config_node_single_token>(after_key));
                    if (after_key->get_token_type() == token_type::EQUALS) {
                        inside_equals = true;
                        ++_equals_count;
                    }

                    next_value = consolidate_values(key_value_nodes);
                    if (!next_value) {
                        next_value = parse_value(next_token_collecting_whitespace(key_value_nodes));
                    }
                }

                key_value_nodes.push_back(next_value);
                if (inside_equals) {
                    --_equals_count;
                }
                last_inside_equals = inside_equals;

                string key = *key_path->get_path().first();
                path remaining = key_path->get_path().remainder();

                if (remaining.empty()) {
                    auto emplace = keys.emplace(move(key), true);
                    if (!emplace.second) {
                        // In strict JSON, dups should be an error; while in
                        // our custom config language, they should be merged
                        // if the value is an object (or substitution that
                        // could become an object).
                        if (_flavor == config_syntax::JSON) {
                            throw parse_exception("JSON does not allow duplicate fields: '" +
                                                  key + "' was already seen");
                        }
                        emplace.first->second = true;
                    }
                } else {
                    if (_flavor == config_syntax::JSON) {
                        throw config_exception("somehow got multi-element path in JSON mode");
                    }
                    keys.insert(make_pair(move(key), true));
                }
                after_comma = false;
                object_nodes.push_back(make_shared<config_node_field>(key_value_nodes));
            }

            if (check_element_separator(object_nodes)) {
                // continue looping
                after_comma = true;
            } else {
                t = next_token_collecting_whitespace(object_nodes);
                if (t->get_token_type() == token_type::CLOSE_CURLY) {
                    if (!had_open_curly) {
                        throw parse_exception(add_quote_suggestion(t->to_string(),
                            "unbalanced close brace '}' with no open brace", last_inside_equals, last_path));
                    }
                    object_nodes.push_back(make_shared<config_node_single_token>(t));
                    break;
                } else if (had_open_curly) {
                    throw parse_exception(add_quote_suggestion(t->to_string(),
                         "Expecting close brace '}' or a comma, got " + t->to_string(), last_inside_equals, last_path));
                } else {
                    if (t->get_token_type() == token_type::END) {
                        put_back(t);
                        break;
                    } else {
                        throw parse_exception(add_quote_suggestion(t->to_string(),
                            "Expecting end of input or a comma, got " + t->to_string(), last_inside_equals, last_path));
                    }
                }
            }
        }
        return make_shared<config_node_object>(object_nodes);
    }

    shared_ptr<config_node_complex_value> parse_context::parse_array() {
        shared_node_list children;
        children.push_back(make_shared<config_node_single_token>(tokens::open_square_token()));
        shared_token t;

        shared_node_value next_value = consolidate_values(children);
        if (next_value) {
            children.push_back(next_value);
        } else {
            t = next_token_collecting_whitespace(children);

            // special case the first element
            if (t->get_token_type() == token_type::CLOSE_SQUARE) {
                children.push_back(make_shared<config_node_single_token>(t));
                return make_shared<config_node_array>(children);
            } else if (is_valid_array_element(t)) {
                next_value = parse_value(t);
                children.push_back(next_value);
            } else {
                throw parse_exception("List should have ']' or a first element after the '[', instead had token: " +
                t->to_string() + " (if you want " + t->to_string() + " to be part of a string value, then"
                "double quote it)");
            }
        }

        // now remaining elements
        while (true) {
            // just after a value
            if (check_element_separator(children)) {
                // comma or newline equivalent consumed
            } else {
                t = next_token_collecting_whitespace(children);
                if (t->get_token_type() == token_type::CLOSE_SQUARE) {
                    children.push_back(make_shared<config_node_single_token>(t));
                    return make_shared<config_node_array>(children);
                } else {
                    throw parse_exception("List should have ended with ']' or had a comma, instead had token: " +
                                          t->to_string() + " (if you want " + t->to_string() + " to be part of a "
                                          "string value, then double quote it)");
                }
            }

            // now just after a comma
            next_value = consolidate_values(children);
            if (next_value) {
                children.push_back(next_value);
            } else {
                t = next_token_collecting_whitespace(children);
                if (is_valid_array_element(t)) {
                    next_value = parse_value(t);
                    children.push_back(next_value);
                } else if (_flavor != config_syntax::JSON && t->get_token_type() == token_type::CLOSE_SQUARE) {
                    // we allow one trailing comma
                    put_back(t);
                } else {
                    throw parse_exception("List should have had a new element after a comma, instead had token: " +
                                          t->to_string() + " (if you want the comma or" + t->to_string() +
                                                  " to be part of a string value, then double quote it)");
                }
            }
        }
    }

    shared_ptr<config_node_root> parse_context::parse() {
        shared_node_list children;
        shared_token t = next_token();
        if (t->get_token_type() == token_type::START) {
            // OK
        } else {
            throw config_exception("token stream did not begin with START, had " + t->to_string());
        }

        t = next_token_collecting_whitespace(children);
        shared_node result = nullptr;
        bool missing_curly = false;
        if (t->get_token_type() == token_type::OPEN_CURLY || t->get_token_type() == token_type::OPEN_SQUARE) {
            result = parse_value(t);
        } else {
            if (_flavor == config_syntax::JSON) {
                if (t->get_token_type() == token_type::END) {
                    throw parse_exception("empty document");
                } else {
                    throw parse_exception("Document must have an object or array at root, unexpected token: " +
                    t->to_string());
                }
            } else {
                // the root object can omit the surrounding braces.
                // this token should be the first field's key, or part
                // of it, so put it back.
                put_back(t);
                missing_curly = true;
                result = parse_object(false);
            }
        }
        // Need to pull the children out of the resulting node so we can keep leading
        // and trailing whitespace if this was a no-brace object. Otherwise, we need to add
        // the result into the list of children.
        auto node_obj = dynamic_pointer_cast<config_node_object>(result);
        if (node_obj && missing_curly) {
            children.insert(children.end(), node_obj->children().begin(), node_obj->children().end());
        } else {
            children.push_back(result);
        }
        t = next_token_collecting_whitespace(children);
        if (t->get_token_type() == token_type::END) {
            if (missing_curly) {
                // If there were no braces, the entire document should be treated as a single object
                return make_shared<config_node_root>(shared_node_list { make_shared<config_node_object>(children) },
                                                     _base_origin);
            } else {
                return make_shared<config_node_root>(children, _base_origin);
            }
        } else {
            throw parse_exception("Document has trailing tokens after first object or array: " + t->to_string());
        }
    }

    shared_node_value parse_context::parse_single_value() {
        shared_token t = next_token();
        if (t->get_token_type() == token_type::START) {
            // OK
        } else {
            throw config_exception("token stream did not being with START, had " + t->to_string());
        }

        t = next_token();
        if (t->get_token_type() == token_type::IGNORED_WHITESPACE || t->get_token_type() == token_type::NEWLINE ||
                is_unquoted_whitespace(t) || t->get_token_type() == token_type::COMMENT) {
            throw parse_exception("The value from with_value_text cannot have leading or trailing newlines, "
                                          "whitespace, or comments");
        }
        if (t->get_token_type() == token_type::END) {
            throw parse_exception("Empty value");
        }
        if (_flavor == config_syntax::JSON) {
            shared_node_value node = parse_value(t);
            t = next_token();
            if (t->get_token_type() == token_type::END) {
                return node;
            } else {
                throw parse_exception("Parsing JSON and the value set in with_value_text was either a concatenation or "
                                      "had trailing whitespace, newlines, or comments");
            }
        } else {
            put_back(t);
            shared_node_list nodes;
            shared_node_value node = consolidate_values(nodes);
            t = next_token();
            if (t->get_token_type() == token_type::END) {
                return node;
            } else {
                throw parse_exception("The value from with_value_text cannot have leading or trailing newlines, "
                "whitespace, or comments");
            }
        }
    }

    bool parse_context::is_valid_array_element(shared_token t) {
        return t->get_token_type() == token_type::VALUE ||
               t->get_token_type() == token_type::OPEN_CURLY ||
               t->get_token_type() == token_type::OPEN_SQUARE ||
               t->get_token_type() == token_type::UNQUOTED_TEXT ||
               t->get_token_type() == token_type::SUBSTITUTION;
    }

}}  // namespace hocon::config_document_parser
