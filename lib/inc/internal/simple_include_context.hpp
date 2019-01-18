#pragma once

#include <hocon/config_include_context.hpp>
#include "parseable.hpp"

namespace hocon {

    class simple_include_context : public config_include_context {
    public:
        // Include context is part of a parseable, so it can always expect a valid parseable reference.
        simple_include_context(parseable const& parseable);
        simple_include_context(parseable const& parseable, shared_full_current fpath);

        // Unused method
        // shared_include_context with_parseable(weak_parseable new_parseable) const;

        shared_parseable relative_to(std::string file_name) const override;
        config_parse_options parse_options() const override;

    public:
        shared_full_current _fpath;
    private:
        parseable const& _parseable;
    };
}  // namespace hocon
