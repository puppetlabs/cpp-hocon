#include <hocon/config_object.hpp>
#include <hocon/config.hpp>
#include <internal/simple_config_origin.hpp>
#include <internal/values/config_delayed_merge_object.hpp>
#include <hocon/config_exception.hpp>
#include <hocon/path.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_object::config_object(shared_origin origin) : config_value(move(origin)) { }

    shared_value config_object::peek_path(path desired_path) const {
        return peek_path(this, move(desired_path));
    }

    shared_value config_object::peek_path(const config_object* self, path desired_path) {
        try {
            path next = desired_path.remainder();
            shared_value v = self->attempt_peek_with_partial_resolve(*desired_path.first());

            if (next.empty()) {
                return v;
            } else {
                if (auto object = dynamic_pointer_cast<const config_object>(v)) {
                    return peek_path(object.get(), next);
                } else {
                    return nullptr;
                }
            }
        } catch (config_exception& ex) {
            throw config_exception(_("{1} has not been resolved, you need to call config::resolve()", desired_path.render()));
        }
    }

    shared_value config_object::peek_assuming_resolved(std::string const& key, path original_path) const {
        try {
            return attempt_peek_with_partial_resolve(key);
        } catch (config_exception& ex) {
            throw config_exception(_("{1} has not been resolved, you need to call config::resolve()", original_path.render()));
        }
    }

    shared_value config_object::new_copy(shared_origin origin) const {
        return new_copy(get_resolve_status(), origin);
    }

    shared_value config_object::construct_delayed_merge(shared_origin origin, std::vector<shared_value> stack) const {
        return make_shared<config_delayed_merge_object>(move(origin), move(stack));
    }

    std::shared_ptr<const config> config_object::to_config() const {
        return make_shared<config>(dynamic_pointer_cast<const config_object>(shared_from_this()));
    }

    config_value::type config_object::value_type() const {
        return config_value::type::OBJECT;
    }

    shared_origin config_object::merge_origins(std::vector<shared_value> const& stack) {
        if (stack.empty()) {
            throw config_exception(_("can't merge origins on empty list"));
        }

        vector<shared_origin> origins;
        shared_origin first_origin = nullptr;

        for (shared_value v : stack) {
            if (first_origin == nullptr) {
                first_origin = v->origin();
            }

            auto cv = dynamic_pointer_cast<const config_object>(v);
            if (cv && cv->get_resolve_status() == resolve_status::RESOLVED && cv->is_empty()) {
                // don't include empty files or the .empty()
                // config in the description, since they are
                // likely to be "implementation details"
            } else {
                origins.push_back(v->origin());
            }
        }

        if (origins.size() == 0) {
            // the configs were all empty, so just use the first one
            origins.push_back(first_origin);
        }

        return simple_config_origin::merge_origins(origins);
    }


}  // namespace hocon
