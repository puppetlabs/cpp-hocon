#pragma once

#include <string>
#include <memory>
#include <hocon/path.hpp>

namespace hocon {

    class substitution_expression : public std::enable_shared_from_this<substitution_expression> {
    public:
        substitution_expression(path the_path, bool optional);

        path get_path() const;
        bool optional() const;

        std::shared_ptr<substitution_expression> change_path(path new_path);

        std::string to_string() const;

    private:
        const path _path;
        const bool _optional;
    };

}  // namespace hocon

