#include <internal/resolve_source.hpp>
#include <hocon/config_exception.hpp>

namespace hocon {

    using namespace std;

    resolve_source::resolve_source(shared_object root)
         : _root(root) { }

    resolve_source resolve_source::push_parent(shared_container parent) const
    {
        // TODO: implement
        throw config_exception("resolve_source::push_parent not yet implemented");
    }

}  // namespace hocon
