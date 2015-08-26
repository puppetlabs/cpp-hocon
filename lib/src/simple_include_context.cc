#include <internal/simple_include_context.hpp>
#include <internal/simple_includer.hpp>

using namespace std;

namespace hocon {

    simple_include_context::simple_include_context(shared_ptr<parseable> parse) : _parseable(move(parse)) { }

    shared_ptr<simple_include_context> simple_include_context::with_parseable(
            shared_ptr<parseable> new_parseable) const {
        return make_shared<simple_include_context>(new_parseable);
    }

    std::shared_ptr<config_parseable> simple_include_context::relative_to(std::string file_name) const {
        if (_parseable) {
            return _parseable->relative_to(file_name);
        } else {
            return nullptr;
        }
    }

    config_parse_options simple_include_context::parse_options() const {
        return simple_includer::clear_for_include(_parseable->options());
    }

}  // namespace hocon
