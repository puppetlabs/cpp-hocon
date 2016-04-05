#include <hocon/config_exception.hpp>
#include <hocon/config_value.hpp>
#include <internal/resolve_context.hpp>
#include <internal/resolve_result.hpp>
#include <internal/resolve_source.hpp>

using namespace std;

namespace hocon {

    resolve_context::resolve_context(config_resolve_options options, path restrict_to_child)
         : _options(move(options)), _restrict_to_child(move(restrict_to_child)) { }

    bool resolve_context::is_restricted_to_child() const
    {
        return !_restrict_to_child.empty();
    }

    config_resolve_options resolve_context::options() const
    {
        return _options;
    }

    resolve_result<shared_value> resolve_context::resolve(shared_value original, resolve_source const& source) const
    {
        memo_key full_key {original, {}};
        memo_key restricted_key = {nullptr, {}};

        shared_value cached;
        auto cached_iter = _memos.find(full_key);
        if (cached_iter != _memos.end()) {
            cached = cached_iter->second;
        }

        if (!cached && is_restricted_to_child()) {
            restricted_key = {original, restrict_to_child()};
            auto cached_iter = _memos.find(full_key);
            if (cached_iter != _memos.end()) {
                cached = cached_iter->second;
            }
        }

        if (cached) {
            return make_resolve_result(*this, cached);
        } else {
            // TODO: cycle detection needs to happen here

            auto result = original->resolve_substitutions(*this, source);
            auto& resolved = result.value;
            auto& with_memo = result.context;

            if (!resolved || resolved->get_resolve_status() == resolve_status::RESOLVED) {
                with_memo = with_memo.memoize(full_key, resolved);
            }  else {
                if (is_restricted_to_child()) {
                    if (restricted_key.value == nullptr && restricted_key.restrict_to_child.empty()) {
                        throw bug_or_broken_exception("restricted_key should not be empty here");
                    }
                    with_memo = with_memo.memoize(restricted_key, resolved);
                } else if (options().get_allow_unresolved()) {
                    with_memo = with_memo.memoize(full_key, resolved);
                } else {
                    throw bug_or_broken_exception("resolve_substitutions() did not give us a resolved object");
                }
            }

            return make_resolve_result(with_memo, resolved);
        }
    }

    path resolve_context::restrict_to_child() const {
        return _restrict_to_child;
    }

    resolve_context resolve_context::restrict(path restrict_to) const {
        if (restrict_to == _restrict_to_child) {
            return *this;
        } else {
            return resolve_context(_options, restrict_to);
        }
    }

    resolve_context resolve_context::unrestricted() const {
        return restrict({});
    }

    shared_value resolve_context::resolve(shared_value value, shared_object root, config_resolve_options options) {
        resolve_source source { root };
        resolve_context context { options, path() };

        return context.resolve(value, source).value;
    }

    resolve_context resolve_context::memoize(const resolve_context::memo_key& key, const shared_value& value) const {
        resolve_context result {_options, _restrict_to_child};
        result._memos = _memos;
        result._memos.emplace(key, value);
        return result;
    }

    std::size_t resolve_context::memo_key_hash::operator()(const hocon::resolve_context::memo_key& k) const {
        // Treat pointer as our hash value
        size_t h = reinterpret_cast<size_t>(k.value.get());
        auto p = k.restrict_to_child;
        while (!p.empty()) {
            h += 41 * (41 + hash<string>()(*p.first()));
            p = p.remainder();
        }
        return h;
    }
}  // namespace hocon
