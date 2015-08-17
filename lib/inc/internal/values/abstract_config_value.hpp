#pragma once

#include <hocon/config_value.hpp>
#include <internal/simple_config_origin.hpp>

namespace hocon {

    enum class resolved_status { RESOLVED, UNRESOLVED };

    class abstract_config_value : public config_value {
    public:
        abstract_config_value(shared_origin origin);

        virtual std::string transform_to_string() const;
        virtual resolved_status resolved() const;

        shared_origin const& origin() const;

    private:
        shared_origin _origin;
    };

    using shared_value = std::shared_ptr<const abstract_config_value>;

}  // namespace hocon
