#pragma once

#include <hocon/config_value.hpp>
#include <hocon/config_list.hpp>
#include <hocon/config_render_options.hpp>
#include <internal/container.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_source.hpp>
#include <internal/exception.hpp>
#include <algorithm>
#include <memory>
#include <vector>

namespace hocon {

    class simple_config_list : public config_list, public container, public std::enable_shared_from_this<simple_config_list> {

    public:

        // This would be useful if we need to iterator over all the values in a list
        // We don't know if we need it yet
        // using iterator = std::vector<shared_value>::const_iterator;

        simple_config_list(shared_origin origin, std::vector<shared_value> value);
        simple_config_list(shared_origin origin, std::vector<shared_value> value, resolve_status status);
        config_value_type value_type() const;
        //unwrapped()
        resolve_status get_resolve_status() const {return _resolved ? resolve_status::RESOLVED : resolve_status::UNRESOLVED;}
        shared_value replace_child(shared_value child, shared_value replacement) const override; 
        bool has_descendant(shared_value descendant) const;

        resolve_result<simple_config_list> resolve_substitutions(std::shared_ptr<resolve_context> context,
                                                                 std::shared_ptr<resolve_source> source) const;
        std::shared_ptr<simple_config_list> relativized(const std::string prefix) const;

        bool contains(shared_value v) const { return std::find(_value.begin(), _value.end(), v) != _value.end(); }
        bool contains_all(std::vector<shared_value>) const;
        shared_value get(int index) const {return _value[index];}

        // YOLO
        int index_of(shared_value v) { 
            auto pos = find(_value.begin(), _value.end(), v); 
            if (pos == _value.end()) {
                return -1;
            } else {
                return pos - _value.begin();
            }
        }


        bool is_empty() const {return _value.empty();}
        int size() const { return _value.size(); }
        std::vector<shared_value> sub_list(int from_index, int to_index) const;

        bool add(shared_value e) const {throw unsupported_operation_exception(exception_string() + "add");}
        void add(int index, shared_value element) const {throw unsupported_operation_exception(exception_string() + "add");}
        bool add_all(std::vector<shared_value>) const {throw unsupported_operation_exception(exception_string() + "add_all");}
        bool add_all(int index, std::vector<shared_value>) const {throw unsupported_operation_exception(exception_string() + "add_all");}
        void clear() const {throw unsupported_operation_exception(exception_string() + "clear");}
        bool remove(shared_value v) const {throw unsupported_operation_exception(exception_string() + "remove");}
        shared_value remove(int index) const {throw unsupported_operation_exception(exception_string() + "remove");}
        bool remove_all(std::vector<shared_value>) const {throw unsupported_operation_exception(exception_string() + "remove_all");}
        bool retain_all(std::vector<shared_value>) const {throw unsupported_operation_exception(exception_string() + "retain_all");}
        shared_value set(int index, shared_value element) const {throw unsupported_operation_exception(exception_string() + "set");}

        const std::shared_ptr<simple_config_list> concatenate(simple_config_list other);
        std::shared_ptr<const config_list> with_origin(shared_origin origin);

        bool operator==(simple_config_list const& other) const;

    protected:
        void render_list(std::string s, int indent, bool atRoot, std::shared_ptr<config_render_options> options);
        std::shared_ptr<simple_config_list> new_copy(shared_origin) {return std::enable_shared_from_this<simple_config_list>::shared_from_this();}

    private:
        static const long _serial_version_UID = 2L;
        const std::vector<shared_value> _value;
        const bool _resolved;

        static const std::string exception_string() {return "config_list is immutable, you can't call simple_config_list.";}

        std::shared_ptr<simple_config_list> modify(no_exceptions_modifier modifier, resolve_status new_resolve_status);
        std::shared_ptr<simple_config_list> modify_may_throw(std::shared_ptr<modifier> modifier, resolve_status new_resolve_status);
        
        class resolve_modifier : public modifier {

        public:
            resolve_modifier(std::shared_ptr<resolve_context> context, std::shared_ptr<resolve_source> source);
            shared_value modify_child_may_throw(std::string key, shared_value v) const override;

        private:
            std::vector<shared_value> _value;
            const std::shared_ptr<resolve_source> _source;
            std::shared_ptr<resolve_context> _context;
       
        };
    };

} //namespace hocon
