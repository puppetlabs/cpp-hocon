#include <hocon/config_object.hpp>
#include <hocon/path.hpp>
#include <internal/values/config_concatenation.hpp>
#include <internal/values/config_string.hpp>
#include <internal/default_transformer.hpp>
#include <internal/values/simple_config_list.hpp>
#include <internal/simple_config_origin.hpp>
#include <hocon/config_exception.hpp>
#include <internal/values/config_string.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>
#include <internal/resolve_context.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    config_concatenation::config_concatenation(shared_origin origin, std::vector<shared_value> pieces) :
            config_value(move(origin)), _pieces(move(pieces)) {
        if (_pieces.size() < 2) {
            throw config_exception(_("Created concatenation with less than 2 items"));
        }

        bool had_unmergeable = false;
        for (shared_value p : _pieces) {
            if (dynamic_pointer_cast<const config_concatenation>(p)) {
                throw config_exception(_("config_concatenation should never be nested"));
            }

            if (dynamic_pointer_cast<const unmergeable>(p)) {
                had_unmergeable = true;
            }
        }

        if (!had_unmergeable) {
            throw config_exception(_("Created concatenation without an unmergeable in it"));
        }
    }

    config_value::type config_concatenation::value_type() const {
        throw not_resolved();
    }

    vector<shared_value> config_concatenation::unmerged_values() const {
        return { shared_from_this() };
    }

    resolve_status config_concatenation::get_resolve_status() const {
        return resolve_status::UNRESOLVED;
    }

    shared_value config_concatenation::replace_child(shared_value const& child, shared_value replacement) const {
        auto new_list = config_value::replace_child_in_list(_pieces, child, replacement);
        if (new_list.empty()) {
            return nullptr;
        } else {
            return make_shared<config_concatenation>(origin(), move(new_list));
        }
    }

    bool config_concatenation::has_descendant(shared_value const &descendant) const {
        return has_descendant_in_list(_pieces, descendant);
    }

    resolve_result<shared_value>
    config_concatenation::resolve_substitutions(resolve_context const& context,
                                                resolve_source const& source) const
    {
        // Right now there's no reason to pushParent here because the
        // content of ConfigConcatenation should not need to replaceChild,
        // but if it did we'd have to do this.
        resolve_context new_context { context };

        vector<shared_value> resolved;
        resolved.reserve(_pieces.size());

        for (auto& p : _pieces) {
            // to concat into a string we have to do a full resolve,
            // so unrestrict the context, then put restriction back afterward
            auto restriction = new_context.restrict_to_child();
            resolve_result<shared_value> result {new_context.unrestricted().resolve(p, source)};
            auto r = result.value;
            new_context = result.context.restrict(restriction);

            if (r == nullptr) {
                // it was optional... omit
            } else {
                resolved.push_back(r);
            }
        }

        // now need to concat everything
        vector<shared_value> joined { consolidate(resolved) };
        // if unresolved is allowed we can just become another
        // ConfigConcatenation
        if (joined.size() > 1 && context.options().get_allow_unresolved()) {
            return make_resolve_result(move(new_context), make_shared<config_concatenation>(origin(), move(joined)));
        } else if (joined.empty()) {
            // we had just a list of optional references using ${?}
            return make_resolve_result(move(new_context), shared_value {});
        } else if (joined.size() == 1) {
            return make_resolve_result(new_context, joined.front());
        } else {
            throw config_exception(_("Bug in the library: resolved list was joined to too many values"));
        }
    }

    vector<shared_value> config_concatenation::consolidate(vector<shared_value> pieces) {
        if (pieces.size() < 2) {
            return pieces;
        } else {
            vector<shared_value> flattened;
            flattened.reserve(pieces.size());
            for (auto& v : pieces) {
                if (auto v_concat = dynamic_pointer_cast<const config_concatenation>(v)) {
                    flattened.insert(flattened.end(), v_concat->_pieces.begin(), v_concat->_pieces.end());
                } else {
                    flattened.push_back(v);
                }
            }

            vector<shared_value> consolidated;
            consolidated.reserve(flattened.size());

            for (auto& v : flattened) {
                if (consolidated.empty()) {
                    consolidated.push_back(v);
                } else {
                    join(consolidated, v);
                }
            }

            return consolidated;
        }
    }

    shared_value config_concatenation::concatenate(std::vector<shared_value> pieces) {
        vector<shared_value> consolidated { consolidate(pieces) };
        if (consolidated.empty()) {
            return nullptr;
        } else if (consolidated.size() == 1) {
            return consolidated.front();
        } else {
            shared_origin merged_origin = simple_config_origin::merge_origins(consolidated);
            return make_shared<config_concatenation>(move(merged_origin), move(consolidated));
        }
    }

    // when you graft a substitution into another object,
    // you have to prefix it with the location in that object
    // where you grafted it; but save prefixLength so
    // system property and env variable lookups don't get
    // broken.
    shared_value config_concatenation::relativized(std::string prefix) const {
        vector<shared_value> new_pieces;
        new_pieces.reserve(_pieces.size());
        for (auto& p : _pieces) {
            new_pieces.push_back(p->relativized(prefix));
        }

        return make_shared<config_concatenation>(origin(), move(new_pieces));
    }

    bool config_concatenation::operator==(config_value const& other) const {
        // note that "origin" is deliberately NOT part of equality
        return equals<config_concatenation>(other, [&](config_concatenation const& o) {
            if (_pieces.size() != o._pieces.size()) { return false; }
            bool result = true;
            for (unsigned long i = 0; i < _pieces.size(); i++) {
                result = *_pieces[i] == *o._pieces[i];
            }
            return result;
        });
    }

    shared_value config_concatenation::new_copy(shared_origin origin) const {
        return make_shared<config_concatenation>(move(origin), _pieces);
    }

    unwrapped_value config_concatenation::unwrapped() const {
        throw config_exception(_("Not resolved, call config::resolve() before attempting to unwrap. See API docs."));
    }

    bool config_concatenation::ignores_fallbacks() const {
        // we can never ignore fallbacks because if a child ConfigReference
        // is self-referential we have to look lower in the merge stack
        // for its value.
        return false;
    }

    void config_concatenation::render(std::string& result, int indent, bool at_root, config_render_options options) const {
        for (auto& p : _pieces) {
            p->render(result, indent, at_root, options);
        }
    }

    config_exception config_concatenation::not_resolved() const {
        return config_exception(_("need to config#resolve(), see the API docs for config#resolve; substitution not resolved"));
    }

    bool config_concatenation::is_ignored_whitespace(shared_value value) {
        auto value_string = dynamic_pointer_cast<const config_string>(value);
        return (value_string && !value_string->was_quoted());
    }

    /**
     * Add left and right, or their merger, to builder.
     */
    void config_concatenation::join(std::vector<shared_value> & builder, shared_value right) {
        auto left = builder.back();

        // check for an object which can be converted to a list
        // (this will be an object with numeric keys, like foo.0, foo.1)
        if (dynamic_pointer_cast<const config_object>(left) && dynamic_pointer_cast<const simple_config_list>(right)) {
            left = default_transformer::transform(left, config_value::type::LIST);
        } else if (dynamic_pointer_cast<const simple_config_list>(left) && dynamic_pointer_cast<const config_object>(right)) {
            right = default_transformer::transform(right, config_value::type::LIST);
        }

        // Since this depends on the type of two instances, I couldn't think
        // of much alternative to an instanceof chain. Visitors are sometimes
        // used for multiple dispatch but seems like overkill.
        shared_value joined = nullptr;

        if (dynamic_pointer_cast<const config_object>(left) && dynamic_pointer_cast<const config_object>(right)) {
            joined = dynamic_pointer_cast<const config_value>(right->with_fallback(left));
        } else if (dynamic_pointer_cast<const simple_config_list>(left) && dynamic_pointer_cast<const simple_config_list>(right)) {
            joined = dynamic_pointer_cast<const simple_config_list>(left)->concatenate(dynamic_pointer_cast<const simple_config_list>(right));
        } else if ((dynamic_pointer_cast<const simple_config_list>(left) || dynamic_pointer_cast<const config_object>(left)) &&
                is_ignored_whitespace(right)) {
            joined = left;
            // it should be impossible that left is whitespace and right is a list or object
        } else if (dynamic_pointer_cast<const config_concatenation>(left) || dynamic_pointer_cast<const config_concatenation>(right)) {
            throw config_exception(_("unflattened config_concatenation"));
        } else if (dynamic_pointer_cast<const unmergeable>(left) || dynamic_pointer_cast<const unmergeable>(right)) {
            // leave joined=null, cannot join
        } else {
            // handle primitive type or primitive type mixed with object or list
            string s1 { left->transform_to_string() };
            string s2 { right->transform_to_string() };
            if (s1.empty() || s2.empty()) {
                throw config_exception(_("Cannot concatenate object or list with a non-object-or-list: {1} and {2} are not compatible", s1, s2));
            } else {
                auto joined_origin = simple_config_origin::merge_origins(left->origin(), right->origin());
                joined = make_shared<config_string>(move(joined_origin), s1 + s2, config_string_type::QUOTED);
            }
        }

        if (joined) {
            builder.pop_back();
            builder.push_back(joined);
        } else {
            builder.push_back(right);
        }
    }
}  // namespace hocon
