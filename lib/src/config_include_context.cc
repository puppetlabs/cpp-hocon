#include <hocon/config_include_context.hpp>
#include <internal/simple_includer.hpp>
#include <internal/parseable.hpp>

using namespace std;

namespace hocon {

    config_include_context::config_include_context(shared_ptr<parseable> parse) : _parseable(move(parse)) { }

    std::shared_ptr<config_parseable> config_include_context::relative_to(std::string file_name) const {
        if (_parseable) {
            return _parseable->relative_to(file_name);
        } else {
            return nullptr;
        }
    }

    config_parse_options config_include_context::parse_options() const {
        return simple_includer::clear_for_include(_parseable->options());
    }

}  // namespace hocon
