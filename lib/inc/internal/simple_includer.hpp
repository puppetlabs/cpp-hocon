#pragma once

#include <hocon/config_includer.hpp>
#include <hocon/config_includer_file.hpp>
#include <hocon/config_parse_options.hpp>

namespace hocon {

    class simple_includer : public config_includer, public config_includer_file {
    public:
        simple_includer(std::shared_ptr<config_includer> fallback);

        std::shared_ptr<config_object> include_file(std::shared_ptr<config_include_context>
                                               context, std::string name) const override;

        static config_parse_options clear_for_include(shared_parse_options options);

    private:
        std::shared_ptr<config_includer> _fallback;
    };

}  // namespace hocon
