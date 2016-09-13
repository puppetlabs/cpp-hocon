#include <hocon/config.hpp>
#include <internal/simple_includer.hpp>
#include <internal/values/simple_config_object.hpp>
#include <internal/parseable.hpp>
#include <hocon/config_exception.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace hocon {

    simple_includer::simple_includer(shared_includer fallback): _fallback(move(fallback)) {}

    shared_includer simple_includer::with_fallback(shared_includer fallback) const {
        auto self = shared_from_this();
        if (self == fallback) {
            throw config_exception(_("Trying to create includer cycle"));
        } else if (_fallback == fallback) {
            return self;
        } else if (_fallback) {
            return make_shared<simple_includer>(_fallback->with_fallback(move(fallback)));
        } else {
            return make_shared<simple_includer>(move(fallback));
        }
    }

    shared_object simple_includer::include(shared_include_context context, string what) const {
        auto obj = include_without_fallback(context, what);

        // now use the fallback includer if any and merge its results
        if (_fallback) {
            return dynamic_pointer_cast<const config_object>(obj->with_fallback(_fallback->include(move(context), move(what))));
        } else {
            return obj;
        }
    }

    shared_object simple_includer::include_without_fallback(shared_include_context context, std::string what) const {
        // TODO: this method deals with URLs and will require Hocon::Impl::Url
        auto source = make_shared<relative_name_source>(context);
        return from_basename(move(source), what, context->parse_options());
    }

    shared_object simple_includer::include_file(shared_include_context context, string what) const {
        auto obj = include_file_without_fallback(context, what);

        // now use the fallback includer if any and merge its result
        if (_fallback && dynamic_pointer_cast<const config_includer_file>(_fallback)) {
            auto fallback_file = dynamic_pointer_cast<const config_includer_file>(_fallback);
            return dynamic_pointer_cast<const config_object>(obj->with_fallback(fallback_file->include_file(move(context), move(what))));
        } else {
            return obj;
        }
    }

    shared_object simple_includer::include_file_without_fallback(shared_include_context context, std::string what) {
        return config::parse_file_any_syntax(move(what), context->parse_options())->root();
    }

    config_parse_options simple_includer::clear_for_include(config_parse_options const& options) {
        // the class loader and includer are inherited, but not this other stuff
        return options.set_syntax(config_syntax::UNSPECIFIED)
                .set_origin_description(make_shared<string>("")).set_allow_missing(true);
    }

    /* this function is a little tricky because there are three places we're
     * trying to use it; for 'include "basename"' in a .conf file, for
     * loading app.{conf,json} from classpath, and for
     * loading app.{conf,json} from the filesystem.
     */
    shared_object simple_includer::from_basename(std::shared_ptr<name_source> source, std::string name,
                                                 config_parse_options options) {
        shared_object obj;
        if (boost::algorithm::ends_with(name, ".conf") || boost::algorithm::ends_with(name, ".json")) {
            auto p = source->name_to_parseable(name, options);
            obj = p->parse(p->options().set_allow_missing(options.get_allow_missing()));
        } else {
            auto conf_handle = source->name_to_parseable(name + ".conf", options);
            auto json_handle = source->name_to_parseable(name + ".json", options);
            bool got_something = false;
            vector<config_exception> fails;

            auto syntax = options.get_syntax();

            obj = simple_config_object::empty(make_shared<simple_config_origin>("empty config"));
            if (syntax == config_syntax::UNSPECIFIED || syntax == config_syntax::CONF) {
                try {
                    auto parse_options = conf_handle->options().set_allow_missing(false).set_syntax(config_syntax::CONF);
                    obj = conf_handle->parse(parse_options);
                } catch (config_exception& ex) {
                    fails.push_back(ex);
                }
            }

            if (syntax == config_syntax::UNSPECIFIED || syntax == config_syntax::JSON) {
                try {
                    auto parse_options = json_handle->options().set_allow_missing(false).set_syntax(config_syntax::JSON);
                    auto parsed = json_handle->parse(parse_options);
                    obj = dynamic_pointer_cast<const config_object>(obj->with_fallback(parsed));

                    got_something = true;
                } catch (config_exception& ex) {
                    fails.push_back(ex);
                }
            }

            if (!options.get_allow_missing() && !got_something) {
                if (fails.empty()) {
                    // this should not happen
                    throw config_exception(_("Should not be reached: nothing found but no exceptions thrown"));
                } else {
                    string sb;
                    for (auto &e : fails) {
                        sb.append(e.what());
                    }
                    throw config_exception(sb);
                }
            } else if (!got_something) {
                // TODO: log that we didn't find `name` with any of our extensions
            }
        }

        return obj;
    }

    shared_ptr<const full_includer> simple_includer::make_full(std::shared_ptr<const config_includer> includer) {
        if (auto inc = dynamic_pointer_cast<const full_includer>(includer)) {
            return inc;
        } else {
            return make_shared<proxy>(includer);
        }
    }

    /** Relative name source */
    relative_name_source::relative_name_source(shared_include_context context) :
            _context(move(context)) {}

    shared_parseable relative_name_source::name_to_parseable(string name,
                                                             config_parse_options parse_options) const {
        auto p = _context->relative_to(name);
        if (p == nullptr) {
            // avoid returning null
            return parseable::new_not_found(name, _("include was not found: '{1}'", name), move(parse_options));
        } else {
            return p;
        }
    }

    /** File name source */
    shared_parseable file_name_source::name_to_parseable(std::string name, config_parse_options parse_options) const {
        return parseable::new_file(move(name), move(parse_options));
    }

    /** Proxy */
    simple_includer::proxy::proxy(std::shared_ptr<const config_includer> delegate) : _delegate(move(delegate)) { }

    shared_includer simple_includer::proxy::with_fallback(shared_includer fallback) const {
        // we never fall back
        return shared_from_this();
    }

    shared_object simple_includer::proxy::include(shared_include_context context, string what) const {
        return _delegate->include(move(context), move(what));
    }

    shared_object simple_includer::proxy::include_file(shared_include_context context, std::string what) const {
        if (auto del = dynamic_pointer_cast<const config_includer_file>(_delegate)) {
            return del->include_file(move(context), move(what));
        } else {
            return include_file_without_fallback(move(context), move(what));
        }
    }

    shared_ptr<const full_includer> simple_includer::proxy::make_full(shared_includer includer) {
        if (auto i = dynamic_pointer_cast<const full_includer>(includer)) {
            return i;
        } else {
            return make_shared<proxy>(move(includer));
        }
    }

}  // namespace hocon
