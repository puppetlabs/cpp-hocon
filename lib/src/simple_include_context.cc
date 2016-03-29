#include <internal/simple_include_context.hpp>
#include <internal/simple_includer.hpp>

using namespace std;

namespace hocon {

    simple_include_context::simple_include_context(parseable const& parse) : _parseable(parse) { }

    shared_parseable simple_include_context::relative_to(std::string file_name) const {
        return _parseable.relative_to(file_name);
    }

    config_parse_options simple_include_context::parse_options() const {
        return simple_includer::clear_for_include(_parseable.options());
    }

}  // namespace hocon
