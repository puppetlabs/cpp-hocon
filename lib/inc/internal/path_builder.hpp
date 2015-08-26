#pragma once

#include <hocon/path.hpp>
#include <stack>

namespace hocon {

    class path_builder {
    public:
        void append_key(std::string key);
        void append_path(path path_to_append);

        /** Returns null if keys is empty. */
        path result();

    private:
        std::stack<std::string> _keys;
    };

}  // namespace hocon
