#include <hocon/config_value.hpp>
#include <internal/simple_config_list.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/config_exception.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_source.hpp>
#include <internal/resolve_result.hpp>
#include <algorithm>

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

    resolve_result<shared_value>
    simple_config_list::resolve_substitutions(resolve_context const& context, resolve_source const& source) const
    {
        if (_resolved == resolve_status::RESOLVED) {
            return resolve_result<shared_value>(context, shared_from_this());
        }

        if (context.is_restricted_to_child()) {
            return resolve_result<shared_value>(context, shared_from_this());
        } else {
            resolve_modifier modifier{context, source.push_parent(dynamic_pointer_cast<const container>(shared_from_this()))};
            auto value = modify_may_throw(modifier, context.options().get_allow_unresolved() ? boost::optional<resolve_status>() : resolve_status::RESOLVED);
            return resolve_result<shared_value>(modifier.context, value);
        }
    }

    std::shared_ptr<const simple_config_list> simple_config_list::relativized(const std::string prefix) const
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
        // TODO
        return {};
    }

}  // namespace hocon
