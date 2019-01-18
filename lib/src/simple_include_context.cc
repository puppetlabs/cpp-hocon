#include <internal/simple_include_context.hpp>
#include <internal/simple_includer.hpp>

using namespace std;

namespace hocon {

    simple_include_context::simple_include_context(parseable const& parse) : config_include_context(), _parseable(parse) { }

    simple_include_context::simple_include_context(parseable const& parseable, shared_full_current fpath)
        : config_include_context(fpath), _parseable(parseable) { }

    shared_parseable simple_include_context::relative_to(std::string file_name) const {
        return _parseable.relative_to(file_name);
    }

    config_parse_options simple_include_context::parse_options() const {
        return simple_includer::clear_for_include(_parseable.options());
    }

}  // namespace hocon
