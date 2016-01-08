#pragma once

namespace hocon {

    class config_exception : public std::runtime_error {
    public:
        config_exception(std::string const& message) : runtime_error(message) { }
        config_exception(std::string const& message, std::exception const& e) : runtime_error(message + " " + e.what()) { }
    };

    class unsupported_operation_exception : public std::runtime_error {
    public:
        unsupported_operation_exception(std::string const& message) : runtime_error(message) { }
    };

}  // namespace hocon
