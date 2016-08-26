#include <hocon/config_value.hpp>
#include <internal/values/simple_config_list.hpp>
#include <internal/simple_config_origin.hpp>
#include <hocon/config_exception.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_source.hpp>
#include <internal/resolve_result.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    struct simple_config_list::resolve_modifier : public modifier {
        resolve_modifier(resolve_context c, resolve_source s) : context(move(c)), source(move(s)) {}

        shared_value modify_child_may_throw(string key, shared_value v) override
        {
            resolve_result<shared_value> result = context.resolve(v, source);
            context = result.context;
            return result.value;
        }

        resolve_context context;
        resolve_source source;
    };

    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value)
            : config_list(move(origin)), _value(move(value)), _resolved(resolve_status_from_values(_value)) { }


    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value,
                                           resolve_status status) : simple_config_list(move(origin), move(value)){
        if (status != _resolved) {
            throw config_exception(_("simple_config_list created with wrong resolve status"));
        }
    }

    shared_value simple_config_list::replace_child(shared_value const& child, shared_value replacement) const
    {
        auto new_list = replace_child_in_list(_value, child, replacement);
        if (new_list.empty()) {
            return nullptr;
        } else {
            return make_shared<simple_config_list>(origin(), move(new_list));
        }
    }

    bool simple_config_list::has_descendant(shared_value const& descendant) const
    {
        return has_descendant_in_list(_value, descendant);
    }

    resolve_result<shared_value>
    simple_config_list::resolve_substitutions(resolve_context const& context, resolve_source const& source) const
    {
        if (_resolved == resolve_status::RESOLVED) {
            return resolve_result<shared_value>(context, shared_from_this());
        }

        if (context.is_restricted_to_child()) {
            return resolve_result<shared_value>(context, shared_from_this());
        } else {
            resolve_modifier mod{context, source.push_parent(dynamic_pointer_cast<const container>(shared_from_this()))};
            auto value = modify_may_throw(mod, context.options().get_allow_unresolved() ? boost::optional<resolve_status>() : resolve_status::RESOLVED);
            return resolve_result<shared_value>(mod.context, value);
        }
    }

    shared_value simple_config_list::relativized(const std::string prefix) const
    {
        no_exceptions_modifier modifier(move(prefix));
        return modify(modifier, get_resolve_status());
    }

    std::shared_ptr<const simple_config_list> simple_config_list::concatenate(shared_ptr<const simple_config_list> other) const
    {
        auto combined_origin = simple_config_origin::merge_origins(origin(), other->origin());
        vector<shared_value> combined;
        combined.reserve(size() + other->size());
        combined.insert(combined.end(), begin(), end());
        combined.insert(combined.end(), other->begin(), other->end());
        return make_shared<simple_config_list>(combined_origin, move(combined));
    }

    shared_value simple_config_list::new_copy(shared_origin origin) const
    {
        // TODO: Copies the list, but the list is immutable so we could share the vector.
        //       Best to deal with in a rewrite that encapsulates shared_ptr and immutability better.
        return make_shared<simple_config_list>(move(origin), _value);
    }

    bool simple_config_list::operator==(config_value const& other) const
    {
        return equals<simple_config_list>(other, [&](simple_config_list const& o) {
            if (size() != o.size()) {
                return false;
            }

            if (equal(begin(), end(), o.begin(),
                      [](shared_value const& a, shared_value const& b) { return a == b; })) {
                return true;
            }

            return equal(begin(), end(), o.begin(), [](shared_value const& a, shared_value const& b) { return *a == *b; });
        });
    }

    void simple_config_list::render(std::string& sb,
                                    int num_indent,
                                    bool at_root,
                                    config_render_options options) const
    {
        if (_value.empty()) {
            sb.append("[]");
        } else {
            sb.push_back('[');
            if (options.get_formatted()) {
                sb.push_back('\n');
            }
            for (auto &v : _value) {
                if (options.get_origin_comments()) {
                    // Could be done more efficiently with a split_iterator, but those are trickier to use with range-for.
                    vector<string> lines;
                    boost::algorithm::split(lines, v->origin()->description(), boost::is_any_of("\n"));
                    for (auto& l : lines) {
                        indent(sb, num_indent+1, options);
                        sb.push_back('#');
                        if (!l.empty()) {
                            sb.push_back(' ');
                        }
                        sb.append(l);
                        sb.push_back('\n');
                    }
                }
                if (options.get_comments()) {
                    for (auto& comment : v->origin()->comments()) {
                        indent(sb, num_indent+1, options);
                        sb.append("# ");
                        sb.append(comment);
                        sb.push_back('\n');
                    }
                }
                indent(sb, num_indent+1, options);

                v->render(sb, num_indent+1, at_root, options);
                sb.push_back(',');
                if (options.get_formatted()) {
                    sb.push_back('\n');
                }
            }
            sb.pop_back();  // chop comma or newline
            if (options.get_formatted()) {
                sb.pop_back();  // chop comma and put back newline
                sb.push_back('\n');
                indent(sb, num_indent, options);
            }
            sb.push_back(']');
        }
    }


    unwrapped_value simple_config_list::unwrapped() const {
        vector<unwrapped_value> values;
        for (auto it = _value.begin(), endIt = _value.end(); it != endIt; ++it) {
            values.emplace_back((*it)->unwrapped());
        }
        return values;
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify(no_exceptions_modifier& modifier,
                               boost::optional<resolve_status> new_resolve_status) const
    {
        // TODO: Do we want similar exception wrapping?
        return modify_may_throw(modifier, new_resolve_status);
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify_may_throw(modifier& modifier,
                                         boost::optional<resolve_status> new_resolve_status) const
    {
        bool init = false;
        vector<shared_value> changed;
        for (auto it = _value.begin(), endIt = _value.end(); it != endIt; ++it) {
            auto modified = modifier.modify_child_may_throw({}, *it);

            // lazy-create the new list if required
            if (changed.empty() && modified != *it) {
                changed.reserve(_value.size());
                changed.insert(changed.end(), _value.begin(), it);
                init = true;
            }

            // once the new list is created, all elements have to go in it.
            // if modify_child returned null, we drop that element.
            if (init && modified) {
                changed.push_back(move(modified));
            }
        }

        if (init) {
            if (new_resolve_status) {
                return make_shared<simple_config_list>(origin(), move(changed), *new_resolve_status);
            } else {
                return make_shared<simple_config_list>(origin(), move(changed));
            }
        } else {
            return dynamic_pointer_cast<const simple_config_list>(shared_from_this());
        }
    }


}  // namespace hocon
