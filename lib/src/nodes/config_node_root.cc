#include <internal/nodes/config_node_root.hpp>
#include <hocon/config_exception.hpp>
#include <internal/nodes/config_node_array.hpp>
#include <internal/nodes/config_node_object.hpp>
#include <internal/path_parser.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_node_root::config_node_root(shared_node_list children, shared_origin origin) :
            config_node_complex_value(move(children)), _origin(move(origin)) { }

    shared_ptr<const config_node_complex_value> config_node_root::new_node(shared_node_list nodes) const {
        throw config_exception(_("Tried to indent a root node"));
    }

    shared_ptr<const config_node_complex_value> config_node_root::value() const {
        for (auto&& node : children()) {
            if (auto complex = dynamic_pointer_cast<const config_node_complex_value>(node)) {
                return complex;
            }
        }
        throw config_exception(_("Root node did not contain a value"));
    }

    shared_ptr<const config_node_root> config_node_root::set_value(std::string desired_path,
                                                                   shared_node_value value,
                                                                   config_syntax flavor) const
    {
        shared_node_list children_copy = children();
        for (size_t i = 0; i < children_copy.size(); i++) {
            auto node = children_copy[i];
            if (dynamic_pointer_cast<const config_node_complex_value>(node)) {
                if (dynamic_pointer_cast<const config_node_array>(node)) {
                    throw config_exception(_("The config document had an array at the root level, and values cannot be modified inside an array"));
                } else if (auto object = dynamic_pointer_cast<const config_node_object>(node)) {
                    if (value == nullptr) {
                        children_copy[i] = object->remove_value_on_path(desired_path, flavor);
                    } else {
                        children_copy[i] = object->set_value_on_path(desired_path, value, flavor);
                    }
                    return make_shared<config_node_root>(children_copy, _origin);
                }
            }
        }
        throw config_exception(_("Root node did not contain a value"));
    }

    bool config_node_root::has_value(string desired_path) const {
        path raw_path = path_parser::parse_path(desired_path);
        shared_node_list children_copy = children();
        for (size_t i = 0; i < children_copy.size(); i++) {
            auto node = children_copy[i];
            if (dynamic_pointer_cast<const config_node_complex_value>(node)) {
                if (dynamic_pointer_cast<const config_node_array>(node)) {
                    throw config_exception(_("The config document had an array at the root level, and values cannot be modified inside an array"));
                } else if (auto object = dynamic_pointer_cast<const config_node_object>(node)) {
                    return object->has_value(raw_path);
                }
            }
        }
        throw config_exception(_("Root node did not contain a value"));
    }

}  // namespace hocon
