#include <internal/simple_includer.hpp>

using namespace std;

namespace hocon {

    config_parse_options simple_includer::clear_for_include(shared_parse_options options) {
        // the class loader and includer are inherited, but not this other stuff
        return options->set_syntax(config_syntax::UNSPECIFIED)
                .set_origin_description(make_shared<string>("")).set_allow_missing(true);
    }

}  // namespace hocon
