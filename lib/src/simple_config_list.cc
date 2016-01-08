#include <hocon/config_value.hpp>
#include <internal/simple_config_list.hpp>
#include <internal/config_exception.hpp>

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

    shared_value simple_config_list::replace_child(shared_value child, shared_value replacement) const
    {
        return {};
    }

    bool simple_config_list::has_descendant(shared_value descendant) const
    {
        return true;
    }

    resolve_result<shared_ptr<const simple_config_list>>
    simple_config_list::resolve_substitutions(std::shared_ptr<resolve_context> context,
                                              std::shared_ptr<resolve_source> source) const
    {
        return {resolve_context(), std::enable_shared_from_this<simple_config_list>::shared_from_this()};
    }

    std::shared_ptr<const simple_config_list> simple_config_list::relativized(const std::string prefix) const
    {
        return {};
    }

    std::vector<shared_value> simple_config_list::sub_list(int from_index, int to_index) const
    {
        return {};
    }

    std::shared_ptr<const simple_config_list> simple_config_list::concatenate(simple_config_list other) const
    {
        return {};
    }

    std::shared_ptr<const config_list> simple_config_list::with_origin(shared_origin origin) const
    {
        return {};
    }

    bool simple_config_list::operator==(simple_config_list const& other) const
    {
        return true;
    }

    void simple_config_list::render_list(std::string s,
                                         int indent,
                                         bool atRoot,
                                         std::shared_ptr<config_render_options> options) const
    {
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify(no_exceptions_modifier modifier,
                               resolve_status new_resolve_status) const
    {
        return {};
    }

    std::shared_ptr<const simple_config_list>
    simple_config_list::modify_may_throw(std::shared_ptr<modifier> modifier,
                                         resolve_status new_resolve_status) const
    {
        return {};
    }

    simple_config_list::resolve_modifier::resolve_modifier(std::shared_ptr<resolve_context> context,
                                                           std::shared_ptr<resolve_source> source)
    {
    }

    shared_value
    simple_config_list::resolve_modifier::modify_child_may_throw(std::string key,
                                                                 shared_value v) const
    {
        return v;
    }

}  // namespace hocon
