#pragma once

#include <hocon/functional_list.hpp>
#include <hocon/types.hpp>

#include <string>
#include <vector>
#include <memory>
#include "export.h"

namespace hocon {

    class LIBCPP_HOCON_EXPORT path {
    public:
        path();
        explicit path(std::string first, path const& remainder);
        explicit path(std::vector<std::string> elements);
        explicit path(std::vector<path> paths_to_concat);

        shared_string first() const;

        /** Returns the path minus the first element, or null if no more elements. */
        path remainder() const;

        /** Returns the path minus the last element or null if we have just one element. */
        path parent() const;

        bool has_remainder() const;
        bool empty() const;
        shared_string last() const;
        path prepend(path prefix);
        int length() const;
        path sub_path(int remove_from_front);
        path sub_path(int start_index, int end_index);
        bool starts_with(path other) const;

        bool operator==(path const& other) const;
        bool operator!=(path const& other) const;

        /** Signals whether quotes and other noise need to be removed/ignored. */
        static bool has_funky_chars(std::string const& s);

        void append_to_string(std::string& base) const;

        /** For debugging. */
        std::string to_string() const;

        /** Human-readable error-message-oriented string representation of path. */
        std::string render() const;

        static path new_key(std::string key);
        static path new_path(std::string path);

    private:
        path(List<shared_string> other_path);

        List<shared_string> _path;
    };

}  // namespace hocon
