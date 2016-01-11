#include <hocon/config_value.hpp>
#include <internal/simple_config_list.hpp>
#include <internal/config_exception.hpp>
#include <algorithm>

using namespace std;

namespace hocon {

    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value)
            : config_list(move(origin)), _value(move(value)), _resolved(resolve_status_from_values(_value)) { }


    simple_config_list::simple_config_list(shared_origin origin, std::vector<shared_value> value,
                                           resolve_status status) : simple_config_list(move(origin), move(value)){
        if (status != _resolved) {
            throw config_exception("simple_config_list created with wrong resolve status");
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

    resolve_result<shared_ptr<const simple_config_list>>
    simple_config_list::resolve_substitutions(std::shared_ptr<resolve_context> context,
                                              std::shared_ptr<resolve_source> source) const
    {
        if (_resolved == resolve_status::RESOLVED) {
            return resolve_result<shared_ptr<const simple_config_list>>(context, dynamic_pointer_cast<const simple_config_list>(shared_from_this()));
        }

        // TODO Implement the rest
        throw config_exception("resolve_substitutions implementation incomplete");
    }

    std::shared_ptr<const simple_config_list> simple_config_list::relativized(const std::string prefix) const
    {
        return modify(make_shared<no_exceptions_modifier>(move(prefix)), get_resolve_status());
    }

    std::shared_ptr<const simple_config_list> simple_config_list::concatenate(shared_ptr<const simple_config_list> other) const
    {
        // TODO merge origins
        vector<shared_value> combined;
        combined.reserve(size() + other->size());
        combined.insert(combined.end(), begin(), end());
        combined.insert(combined.end(), other->begin(), other->end());
        return make_shared<simple_config_list>(origin(), move(combined));
    }

    std::shared_ptr<const config_list> simple_config_list::with_origin(shared_origin origin) const
    {
        // TODO: implement config_value::with_origin
        // return config_value::with_origin(move(origin));
        return {};
    }

    bool simple_config_list::operator==(simple_config_list const& other) const
    {
        if (size() != other.size()) {
            return false;
        }

        if (equal(begin(), end(), other.begin(),
                  [](shared_value const& a, shared_value const& b) { return a == b; })) {
            return true;
        }

        // TODO: implement config_value::operator==
        // return equal(begin(), end(), other.begin(), [](shared_value &a, shared_value &b) { return *a == *b; });
        return false;
    }

    void simple_config_list::render_list(std::string s,
                                         int indent,
                                         bool atRoot,
                                         std::shared_ptr<config_render_options> options) const
    {
        // TODO: Investigate config_value::render with similar function signature.
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify(std::shared_ptr<no_exceptions_modifier> modifier,
                               resolve_status new_resolve_status) const
    {
        // TODO: Do we want similar exception wrapping?
        return modify_may_throw(modifier, new_resolve_status);
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify_may_throw(std::shared_ptr<modifier> modifier,
                                         resolve_status new_resolve_status) const
    {
        // TODO
        return {};
    }

    simple_config_list::resolve_modifier::resolve_modifier(std::shared_ptr<resolve_context> context,
                                                           std::shared_ptr<resolve_source> source)
    {
        // TODO
    }

    shared_value
    simple_config_list::resolve_modifier::modify_child_may_throw(std::string key,
                                                                 shared_value v) const
    {
        // TODO
        return v;
    }

}  // namespace hocon
