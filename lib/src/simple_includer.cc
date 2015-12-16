#include <internal/simple_includer.hpp>
#include <internal/parseable.hpp>

using namespace std;

namespace hocon {

    config_parse_options simple_includer::clear_for_include(shared_parse_options options) {
        // the class loader and includer are inherited, but not this other stuff
        return options->set_syntax(config_syntax::UNSPECIFIED)
                .set_origin_description(make_shared<string>("")).set_allow_missing(true);
    }

    /** Relative name source */
    relative_name_source::relative_name_source(shared_ptr<config_include_context> context) :
            _context(move(context)) {}

    shared_ptr<config_parseable> relative_name_source::name_to_parseable(string name,
                                                                         shared_parse_options parse_options) const {
        auto p = _context->relative_to(name);
        if (p == nullptr) {
            // avoid returning null
            return make_shared<parseable_not_found>(parseable::new_not_found(name,
                                                                             "include was not found: '" + name + "'",
                                                                             parse_options));
        } else {
            return p;
        }
    }

}  // namespace hocon
