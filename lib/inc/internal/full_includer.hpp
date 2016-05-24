#pragma once

#include <hocon/config_includer.hpp>
#include <hocon/config_includer_file.hpp>


namespace hocon {

    class full_includer : public config_includer, public config_includer_file {
        // This is a faithful port of the Java fullIncluder interface
    };

}  // namespace hocon
