#include <internal/config_parser.hpp>
#include <hocon/config_exception.hpp>
#include <hocon/config_object.hpp>
#include <internal/tokens.hpp>
#include <internal/simple_includer.hpp>
#include <internal/substitution_expression.hpp>
#include <internal/nodes/config_node_comment.hpp>
#include <internal/nodes/config_node_complex_value.hpp>
#include <internal/nodes/config_node_simple_value.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/nodes/config_node_array.hpp>
#include <internal/nodes/config_node_concatenation.hpp>
#include <internal/nodes/config_node_field.hpp>
#include <internal/nodes/config_node_include.hpp>
#include <internal/values/config_concatenation.hpp>
#include <internal/values/config_reference.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/values/simple_config_list.hpp>
#include <leatherman/locale/locale.hpp>
#include <iostream>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

namespace hocon { namespace config_parser {
    using namespace std;

    shared_value parse(shared_ptr<const config_node_root> document,
            shared_origin origin,
            config_parse_options options,
            shared_include_context include_context)
    {
        parse_context context {options.get_syntax(), origin, document,
                               simple_includer::make_full(options.get_includer()), include_context};
        return context.parse();
    }

    parse_context::parse_context(config_syntax flavor, shared_origin origin, shared_ptr<const config_node_root> document,
            shared_ptr<const full_includer> includer, shared_include_context include_context) :
        _line_number(1), _document(document), _includer(includer), _include_context(include_context),
        _flavor(flavor), _base_origin(origin), array_count(0)
    {}

    shared_origin parse_context::line_origin() const {
        return _base_origin->with_line_number(_line_number);
    }

    path parse_context::full_current_path() const {
        // pathStack has top of stack at front
        if (_path_stack.empty()) {
            throw bug_or_broken_exception(_("Bug in parser; tried to get current path when at root"));
        } else {
            return path {_path_stack.front()};
        }
    }

    shared_value parse_context::parse_value(shared_node_value n, vector<string>& comments)
    {
        auto starting_array_count = array_count;

        shared_value v = [&]() -> shared_value {
            if (auto val = dynamic_pointer_cast<const config_node_simple_value>(n)) {
                return val->get_value();
            } else if (auto val = dynamic_pointer_cast<const config_node_object>(n)) {
                return parse_object(val);
            } else if (auto val = dynamic_pointer_cast<const config_node_array>(n)) {
                return parse_array(val);
            } else if (auto val = dynamic_pointer_cast<const config_node_concatenation>(n)) {
                return parse_concatenation(val);
            } else {
                auto &deref_n = *n;
                throw parse_exception(*line_origin(),
                                      _("Expecting a value but got wrong node type: {1}", typeid(deref_n).name()));
            }
        }();

        if (!comments.empty()) {
            auto old_origin = dynamic_pointer_cast<const simple_config_origin>(v->origin());
            if (!old_origin) {
                throw bug_or_broken_exception(_("origin should be a simple_config_origin"));
            }
            v = v->with_origin(old_origin->prepend_comments(move(comments)));
        }

        if (array_count != starting_array_count) {
            throw bug_or_broken_exception(_("Bug in config parser: unbalanced array count"));
        }
        return v;
    }

    shared_object parse_context::create_value_under_path(path p, shared_value value) {
        // for path foo.bar, we are creating { "foo" : { "bar" : value } }
        vector<shared_string> keys;

        shared_string key = p.first();
        path remaining = p.remainder();
        while (key != nullptr) {
            keys.push_back(key);
            if (remaining.empty()) {
                break;
            } else {
                key = remaining.first();
                remaining = remaining.remainder();
            }
        }

        auto current = keys.end();
        current--;
        auto new_value = unordered_map<string, shared_value>({ {**current, value} });
        shared_object obj = make_shared<simple_config_object>(value->origin()->with_comments(vector<string>{}),
                                                              new_value);

        while (current != keys.begin()) {
            current--;
            new_value = unordered_map<string, shared_value>({ {**current, obj} });
            obj = make_shared<simple_config_object>(value->origin()->with_comments(vector<string>{}), new_value);
        }

        return obj;
    }

    void parse_context::parse_include(unordered_map<string, shared_value> & values,
                                      shared_ptr<const config_node_include> n) {
        shared_object obj;
        switch (n->kind()) {
            case config_include_kind::FILE:
                obj = dynamic_pointer_cast<const config_object>(_includer->include_file(_include_context, n->name()));
                break;
            case config_include_kind::CLASSPATH:
                // TODO: implement include_resource (?)
                throw config_exception(_("full_includer::include_resource not implemented"));
                break;
            case config_include_kind::HEURISTIC:
                obj = dynamic_pointer_cast<const config_object>(_includer->include(_include_context, n->name()));
                break;
            default:
                throw config_exception(_("should not be reached"));
                break;
        }

        // we really should make this work, but for now throwing an
        // exception is better than producing an incorrect result.
        // See https://github.com/typesafehub/config/issues/160
        if (array_count > 0 && obj->get_resolve_status() != resolve_status::RESOLVED) {
            throw config_exception(leatherman::locale::translate("Due to current limitations of the config parser, when an include statement is nested inside a list value,\n${} substitutions inside the included file cannot be resolved correctly. Either move the include outside of the list value or\nremove the ${} statements from the included file."));
        }

        if (!_path_stack.empty()) {
            auto prefix = full_current_path();
            obj = dynamic_pointer_cast<const config_object>(obj->relativized(prefix.to_string()));
        }

        for (auto &pair : *obj) {
            auto &key = pair.first;
            auto &v = pair.second;
            auto iter = values.find(key);
            if (iter != values.end()) {
                auto existing = values[key];
                values[key] = dynamic_pointer_cast<const config_value>(v->with_fallback(existing));
            } else {
                values.emplace(key, v);
            }
        }
    }

    shared_object parse_context::parse_object(shared_node_object n)
    {
        unordered_map<string, shared_value> values;
        auto object_origin = line_origin();
        bool last_was_newline = false;

        auto nodes = n->children();
        vector<string> comments;
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto node = nodes.at(i);
            if (auto comment = dynamic_pointer_cast<const config_node_comment>(node)) {
                last_was_newline = false;
                comments.push_back(comment->comment_text());
            } else if (auto singletoken = dynamic_pointer_cast<const config_node_single_token>(node)) {
                if (tokens::is_newline(singletoken->get_token())) {
                    _line_number++;
                    if (last_was_newline) {
                        // Drop all comments if there was a blank line and start a new comment block
                        comments.clear();
                    }
                    last_was_newline = true;
                }
            } else if (auto include = dynamic_pointer_cast<const config_node_include>(node)) {
                if (_flavor != config_syntax::JSON) {
                    parse_include(values, include);
                    last_was_newline = false;
                }
            } else if (auto field = dynamic_pointer_cast<const config_node_field>(node)) {
                last_was_newline = false;
                auto path = field->path()->get_path();
                auto field_comments = field->comments();
                comments.insert(comments.end(), field_comments.begin(), field_comments.end());

                // path must be on-stack while we parse the value
                _path_stack.push_back(path);
                if (field->separator() == tokens::plus_equals_token()) {
                    // we really should make this work, but for now throwing
                    // an exception is better than producing an incorrect
                    // result. See
                    // https://github.com/typesafehub/config/issues/160
                    if (array_count > 0) {
                        throw parse_exception(*line_origin(), leatherman::locale::translate("Due to current limitations of the config parser, += does not work nested inside a list. += expands to a ${} substitution and the path in ${} cannot currently refer to list elements. You might be able to move the += outside of the list and then refer to it from inside the list with ${}."));
                    }

                    // because we will put it in an array after the fact so
                    // we want this to be incremented during the parseValue
                    // below in order to throw the above exception.
                    array_count += 1;
                }

                auto value_node = field->get_value();
                // comments from the key token go to the value token
                auto new_value = parse_value(value_node, comments);

                if (field->separator() == tokens::plus_equals_token()) {
                    array_count -= 1;

                    vector<shared_value> concat;
                    concat.reserve(2);
                    auto previous_ref = make_shared<config_reference>(new_value->origin(), make_shared<substitution_expression>(full_current_path(), true));
                    auto list = make_shared<simple_config_list>(new_value->origin(), vector<shared_value>({new_value}));
                    concat.push_back(previous_ref);
                    concat.push_back(list);
                    new_value = config_concatenation::concatenate(concat);
                }

                // Grab any trailing comments on the same line
                if (i < nodes.size() - 1) {
                    ++i;
                    while (i < nodes.size()) {
                        if (auto comment = dynamic_pointer_cast<const config_node_comment>(nodes.at(i))) {
                            auto old_origin = dynamic_pointer_cast<const simple_config_origin>(new_value->origin());
                            if (!old_origin) {
                                throw bug_or_broken_exception(_("expected origin to be simple_config_origin"));
                            }
                            new_value = new_value->with_origin(old_origin->append_comments({comment->comment_text()}));
                            break;
                        } else if (auto curr = dynamic_pointer_cast<const config_node_single_token>(nodes.at(i))) {
                            if (curr->get_token() == tokens::comma_token() ||
                                tokens::is_ignored_whitespace(curr->get_token())) {
                                // keep searching, as there could still be a comment
                            } else {
                                --i;
                                break;
                            }
                        } else {
                            --i;
                            break;
                        }
                        ++i;
                    }
                }

                _path_stack.pop_back();

                auto key = path.first();
                auto remaining = path.remainder();

                if (remaining.empty()) {
                    auto existing = values.find(*key);
                    if (existing != values.end()) {
                        // In strict JSON, dups should be an error; while in
                        // our custom config language, they should be merged
                        // if the value is an object (or substitution that
                        // could become an object).

                        if (_flavor == config_syntax::JSON) {
                            throw parse_exception(*line_origin(), _("JSON does not allow duplicate fields: '{1}' was already seen at {2}", *key, existing->second->origin()->description()));
                        } else {
                            new_value = dynamic_pointer_cast<const config_value>(new_value->with_fallback(existing->second));
                            assert(new_value);
                        }
                    }
                    values[*key] = new_value;
                } else {
                    if (_flavor == config_syntax::JSON) {
                        throw new bug_or_broken_exception(_("somehow got multi-element path in JSON mode"));
                    }

                    shared_object obj = create_value_under_path(remaining, new_value);
                    auto existing = values.find(*key);
                    if (existing != values.end()) {
                        obj = dynamic_pointer_cast<const config_object>(obj->with_fallback(existing->second));
                        assert(obj);
                    }
                    values[*key] = obj;
                }
            }
        }

        return make_shared<simple_config_object>(object_origin, move(values));
    }

    static shared_ptr<const simple_config_origin> as_origin(shared_origin o) {
        auto simple_o = dynamic_pointer_cast<const simple_config_origin>(o);
        if (!simple_o) {
            throw bug_or_broken_exception(_("origin was not a simple_config_origin"));
        }
        return simple_o;
    }

    shared_value parse_context::parse_array(shared_node_array n) {
        ++array_count;

        auto array_origin = line_origin();
        auto values = vector<shared_value>();

        bool last_was_new_line = false;
        auto comments = vector<string>();

        shared_value v;

        for (auto node : n->children()) {
            if (auto comment = dynamic_pointer_cast<const config_node_comment>(node)) {
                comments.push_back(comment->comment_text());
                last_was_new_line = false;
            } else if (auto singletoken = dynamic_pointer_cast<const config_node_single_token>(node)) {
                if (tokens::is_newline(singletoken->get_token())) {
                    _line_number++;
                    if (last_was_new_line && v == nullptr) {
                        comments.clear();
                    } else if (v) {
                        values.push_back(v->with_origin(as_origin(v->origin())->append_comments(move(comments))));
                        comments.clear();
                        v = nullptr;
                    }
                    last_was_new_line = true;
                }
            } else if (auto value = dynamic_pointer_cast<const abstract_config_node_value>(node)) {
                last_was_new_line = false;
                if (v) {
                    values.push_back(v->with_origin(as_origin(v->origin())->append_comments(move(comments))));
                    comments.clear();
                }
                v = parse_value(value, comments);
            }
        }

        // There shouldn't be any comments at this point, but add them just in case
        if (v) {
            values.push_back(v->with_origin(as_origin(v->origin())->append_comments(move(comments))));
        }
        --array_count;
        return make_shared<simple_config_list>(move(array_origin), move(values));
    }

    shared_value parse_context::parse_concatenation(shared_node_concatenation n) {
        if (_flavor == config_syntax::JSON) {
            throw bug_or_broken_exception(_("Found a concatenation node in JSON"));
        }

        vector<shared_value> values;
        for (auto& node : n->children()) {
            if (auto value_node = dynamic_pointer_cast<const abstract_config_node_value>(node)) {
                vector<string> comments;
                values.push_back(parse_value(value_node, comments));
            }
        }

        return config_concatenation::concatenate(move(values));
    }

    shared_value parse_context::parse()
    {
        shared_value result;
        vector<string> comments;
        bool last_was_newline = false;

        for (auto&& node : _document->children()) {
            if (auto ptr = dynamic_pointer_cast<const config_node_comment>(node)) {
                comments.push_back(ptr->comment_text());
                last_was_newline = false;
            } else if (auto ptr = dynamic_pointer_cast<const config_node_single_token>(node)) {
                auto t = ptr->get_token();
                if (t->get_token_type() == token_type::NEWLINE) {
                    ++_line_number;
                    if (last_was_newline && !result) {
                        comments.clear();
                    } else if (result) {
                        auto origin = dynamic_pointer_cast<const simple_config_origin>(result->origin());
                        assert(origin);
                        result = result->with_origin(origin->append_comments(move(comments)));
                        break;
                    }
                    last_was_newline = true;
                }
            } else if (auto ptr = dynamic_pointer_cast<const config_node_complex_value>(node)) {
                result = parse_value(ptr, comments);
                last_was_newline = false;
            }
        }

        return result;
    }
}}  // namespace hocon::config_parser
