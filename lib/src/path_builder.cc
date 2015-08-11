#include <internal/path_builder.hpp>

using namespace std;

namespace hocon {

    void path_builder::append_key(string key) {
        _keys.push(key);
    }

    void path_builder::append_path(path path_to_append) {
        if (!path_to_append.first()) {
            return;
        }
        string first = *path_to_append.first();
        path remainder = path_to_append;
        while (true) {
            _keys.push(first);
            if (remainder.has_remainder()) {
                remainder = remainder.remainder();
                first = *remainder.first();
            } else {
                break;
            }
        }
    }

    path path_builder::result() {
        path remainder;
        while (!_keys.empty()) {
            string key = _keys.top();
            _keys.pop();
            remainder = path(key, move(remainder));
        }
        return remainder;
    }

}  // namesapce hocon
