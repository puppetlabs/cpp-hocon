#include <internal/nodes/config_node_object.hpp>
#include <internal/nodes/config_node_field.hpp>
#include <internal/nodes/config_node_single_token.hpp>
#include <internal/tokens.hpp>
#include <internal/path_parser.hpp>
#include <internal/nodes/config_node_include.hpp>
#include <boost/range/iterator_range_core.hpp>

using namespace std;

namespace hocon {

    config_node_object::config_node_object(shared_node_list children) :
            config_node_complex_value(move(children)) { }

    shared_ptr<const config_node_complex_value> config_node_object::new_node(shared_node_list nodes) const {
        return make_shared<config_node_object>(move(nodes));
    }

    bool config_node_object::has_value(path desired_path) const {
        for (auto&& node : children()) {
            if (auto field = dynamic_pointer_cast<const config_node_field>(node)) {
                path key = field->path()->get_path();
                if (key == desired_path || key.starts_with(desired_path)) {
                    return true;
                } else if (desired_path.starts_with(key)) {
                    if (auto object = dynamic_pointer_cast<const config_node_object>(field->get_value())) {
                        path remaining_path = desired_path.sub_path(key.length());
                        if (object->has_value(remaining_path)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    shared_ptr<const config_node_object> config_node_object::change_value_on_path(path desired_path, shared_node_value value,
                                                                            config_syntax flavor) const
    {
        shared_node_list children_copy = children();
        bool seen_non_matching = false;
        shared_node_value value_copy = value;
        for (int i = static_cast<int>(children_copy.size() - 1); i >= 0; i--) {
            auto&& child = children_copy[i];
            if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(child)) {
                shared_token t = single_token->get_token();
                // Ensure that, when we are removing settings in JSON, we don't end up with a trailing comma
                if (flavor == config_syntax::JSON && !seen_non_matching && t == tokens::comma_token()) {
                    children_copy.erase(children_copy.begin() + i);
                }
                continue;
            } else if (dynamic_pointer_cast<const config_node_field>(child) == nullptr) {
                continue;
            }
            auto field = dynamic_pointer_cast<const config_node_field>(child);
            path key = field->path()->get_path();

            // Delete all multi-element paths that start with the desired path, since technically they are duplicates
            if ((value_copy == nullptr && key == desired_path) ||
                    (key.starts_with(desired_path) && !(key == desired_path)))
            {
                children_copy.erase(children_copy.begin() + i);
                //  Remove any whitespace or commas after the deleted setting
                for (size_t j = i; j < children_copy.size(); j++) {
                    if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(children_copy[j])) {
                        shared_token t = single_token->get_token();
                        if (t->get_token_type() == token_type::IGNORED_WHITESPACE || t == tokens::comma_token()) {
                            children_copy.erase(children_copy.begin() + j);
                            j--;
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } else if (key == desired_path) {
                seen_non_matching = true;
                shared_node_value indented_value;
                shared_node before = i - 1 > 0 ? children_copy[i - 1] : nullptr;
                if (auto complex = dynamic_pointer_cast<const config_node_complex_value>(value)) {
                    if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(before)) {
                        if (single_token->get_token()->get_token_type() == token_type::IGNORED_WHITESPACE) {
                            indented_value = complex->indent_text(before);
                        } else {
                            indented_value = value;
                        }
                    } else {
                        indented_value = value;
                    }
                } else {
                    indented_value = value;
                }
                children_copy[i] = field->replace_value(indented_value);
                value_copy = nullptr;
            } else if (desired_path.starts_with(key)) {
                seen_non_matching = true;
                if (auto node_object = dynamic_pointer_cast<const config_node_object>(field->get_value())) {
                    path remaining_path = desired_path.sub_path(key.length());
                    children_copy[i] = field->replace_value(
                            node_object->change_value_on_path(remaining_path, value_copy, flavor));
                    if (value_copy != nullptr && !(*field == *children()[i])) {
                        value_copy = nullptr;
                    }
                }
            } else {
                seen_non_matching = true;
            }
        }
        return make_shared<config_node_object>(children_copy);
    }

    shared_ptr<const config_node_object> config_node_object::set_value_on_path(string desired_path, shared_node_value value,
                                                                            config_syntax flavor) const {
        config_node_path path = path_parser::parse_path_node(desired_path, flavor);
        return set_value_on_path(path, value, flavor);
    }

    shared_ptr<const config_node_object> config_node_object::set_value_on_path(config_node_path desired_path, shared_node_value value,
                                                                         config_syntax flavor) const {
        auto node = change_value_on_path(desired_path.get_path(), value, flavor);

        // If desired path did not exist, create it
        if (!node->has_value(desired_path.get_path())) {
            return node->add_value_on_path(desired_path, value, flavor);
        }
        return node;
    }

    shared_node_list config_node_object::indentation() const {
        bool seen_new_line = false;
        shared_node_list indentation;
        if (children().empty()) {
            return indentation;
        }
        for (size_t i = 0; i < children().size(); i++) {
            if (!seen_new_line) {
                if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(children()[i])) {
                    if (single_token->get_token()->get_token_type() == token_type::NEWLINE) {
                        seen_new_line = true;
                        indentation.push_back(make_shared<config_node_single_token>(make_shared<line>(nullptr)));
                    }
                }
            } else {
                if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(children()[i])) {
                    if (single_token->get_token()->get_token_type() == token_type::IGNORED_WHITESPACE &&
                            (i + 1) < children().size()) {
                        if (dynamic_pointer_cast<const config_node_field>(children()[i + 1]) ||
                                dynamic_pointer_cast<const config_node_include>(children()[i + 1])) {
                            indentation.push_back(children()[i]);
                            return indentation;
                        }
                    }
                }
            }
        }
        if (indentation.empty()) {
            indentation.push_back(make_shared<config_node_single_token>(make_shared<ignored_whitespace>(nullptr, " ")));
        } else {
            // Calculate the indentation of the ending curly brace to get the indentation of the root
            shared_node last = children().back();
            auto single_token = dynamic_pointer_cast<const config_node_single_token>(last);
            if (single_token && single_token->get_token()->get_token_type() == token_type::CLOSE_CURLY) {
                shared_node before_last = children()[children().size() - 2];
                string indent = "";
                auto single = dynamic_pointer_cast<const config_node_single_token>(before_last);
                if (single && single->get_token()->get_token_type() == token_type::IGNORED_WHITESPACE) {
                    indent = single->get_token()->token_text();
                }
                indent += "  ";
                indentation.push_back(make_shared<config_node_single_token>(
                        make_shared<ignored_whitespace>(nullptr, indent)));
                return indentation;
            }
        }
        // The object has no curly braces and is at the root level, so don't indent
        return indentation;
    }

    shared_ptr<const config_node_object> config_node_object::add_value_on_path(config_node_path desired_path,
                                                                         shared_node_value value,
                                                                         config_syntax flavor) const
    {
        path raw_path = desired_path.get_path();
        shared_node_list children_copy = children();
        shared_node_list indent = indentation();

        // If the value we're inserting is a complex value, we'll need to indent it for insertion
        shared_node_value indented_value;
        auto complex = dynamic_pointer_cast<const config_node_complex_value>(value);
        if (complex && !indent.empty()) {
            indented_value = complex->indent_text(indent.back());
        } else {
            indented_value = value;
        }
        bool same_line = true;
        if (indent.size() > 0) {
            if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(indent.front())) {
                same_line = (single_token->get_token()->get_token_type() != token_type::NEWLINE);
            }
        }

        // If the path is of length greater than one, see if the value needs to be added further down
        if (raw_path.length() > 1) {
            for (int i = children().size() - 1; i >= 0; i--) {
                if (auto field = dynamic_pointer_cast<const config_node_field>(children()[i])) {
                    path key = field->path()->get_path();
                    if (raw_path.starts_with(key)) {
                        if (auto object = dynamic_pointer_cast<const config_node_object>(field->get_value())) {
                            config_node_path remaining_path = desired_path.sub_path(key.length());
                            children_copy[i] = field->replace_value(object->add_value_on_path(
                                                         remaining_path, value, flavor));
                            return make_shared<config_node_object>(children_copy);
                        }
                    }
                }
            }
        }

        // Otherwise, construct the new setting
        bool starts_with_brace = !children().empty();
        if (auto single_token = dynamic_pointer_cast<const config_node_single_token>(children().front())) {
            starts_with_brace = starts_with_brace && single_token->get_token() == tokens::open_curly_token();
        } else {
            starts_with_brace = false;
        }

        shared_node_list new_nodes;
        new_nodes.insert(new_nodes.end(), indent.begin(), indent.end());
        new_nodes.push_back(make_shared<config_node_path>(desired_path.first()));
        new_nodes.push_back(make_shared<config_node_single_token>(make_shared<ignored_whitespace>(nullptr, " ")));
        new_nodes.push_back(make_shared<config_node_single_token>(tokens::colon_token()));
        new_nodes.push_back(make_shared<config_node_single_token>(make_shared<ignored_whitespace>(nullptr, " ")));

        if (raw_path.length() == 1) {
            new_nodes.push_back(indented_value);
        } else {
            // If the path is of length greater than one add the required new objects along the path
            shared_node_list new_object_nodes;
            new_object_nodes.push_back(make_shared<config_node_single_token>(tokens::open_curly_token()));
            if (indent.empty()) {
                new_object_nodes.push_back(make_shared<config_node_single_token>(make_shared<line>(nullptr)));
            }
            new_object_nodes.insert(new_object_nodes.end(), indent.begin(), indent.end());
            new_object_nodes.push_back(make_shared<config_node_single_token>(tokens::close_curly_token()));
            config_node_object new_object { new_object_nodes };
            new_nodes.push_back(new_object.add_value_on_path(desired_path.sub_path(1), indented_value, flavor));
        }

        // Combine these cases so that we only have to iterate once
        if (flavor == config_syntax::JSON || starts_with_brace || same_line) {
            for (int i = static_cast<int>(children_copy.size() - 1); i >= 0; i--) {
                // If we are in JSON or are adding a setting on the same line,
                // we need to add a comma to the last setting
                if ((flavor == config_syntax::JSON || same_line) &&
                        dynamic_pointer_cast<const config_node_field>(children_copy[i])) {
                    if ((i + 1) >= static_cast<int>(children_copy.size()) ||
                            !contains_token(children_copy[i + 1], token_type::COMMA)) {
                        children_copy.insert(children_copy.begin() + i + 1,
                                             make_shared<config_node_single_token>(tokens::comma_token()));
                        break;
                    }
                }

                // Add the value into the copy of the children map, keeping any whitespace/newlines
                // before the close curly brace
                if (starts_with_brace && contains_token(children_copy[i], token_type::CLOSE_CURLY)) {
                    auto previous = children_copy[i - 1];
                    if (contains_token(previous, token_type::NEWLINE)) {
                        children_copy.insert(children_copy.begin() + i - 1, make_shared<config_node_field>(new_nodes));
                        i--;
                    } else if (contains_token(previous, token_type::IGNORED_WHITESPACE)) {
                        auto before_prev = children_copy[i - 2];
                        if (same_line) {
                            children_copy.insert(children_copy.begin() + i - 1, make_shared<config_node_field>(new_nodes));
                            i--;
                        } else if (contains_token(before_prev, token_type::NEWLINE)) {
                            children_copy.insert(children_copy.begin() + i - 2,
                                                 make_shared<config_node_field>(new_nodes));
                            i -= 2;
                        } else {
                            children_copy.insert(children_copy.begin() + i, make_shared<config_node_field>(new_nodes));
                        }
                    } else {
                        children_copy.insert(children_copy.begin() + i, make_shared<config_node_field>(new_nodes));
                    }
                }
            }
        }

        if (!starts_with_brace) {
            if (!children_copy.empty() && contains_token(children_copy.back(), token_type::NEWLINE)) {
                children_copy.insert(children_copy.end() - 1, make_shared<config_node_field>(new_nodes));
            } else {
                children_copy.push_back(make_shared<config_node_field>(new_nodes));
            }
        }
        return make_shared<config_node_object>(children_copy);
    }

    shared_ptr<const config_node_object> config_node_object::remove_value_on_path(std::string desired_path,
                                                                                  config_syntax flavor) const
    {
        path raw_path = path_parser::parse_path_node(desired_path, flavor).get_path();
        return change_value_on_path(raw_path, nullptr, flavor);
    }

    bool config_node_object::contains_token(shared_node node, token_type type) {
        auto single = dynamic_pointer_cast<const config_node_single_token>(node);
        if (single) {
            return single->get_token()->get_token_type() == type;
        }
        return false;
    }

}  // namesapce hocon
