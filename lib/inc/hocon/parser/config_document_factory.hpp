#pragma once

#include "config_document.hpp"
#include <hocon/config_parse_options.hpp>
#include "../export.h"

/**
 * Factory for creating config_document instances.
 */
namespace hocon { namespace config_document_factory {

    /**
     * Parses a file into a config_document instance.
     *
     * @param file
     *       the file to parse
     * @param options
     *       parse options to control how the file is interpreted
     * @return the parsed configuration
     */
    LIBCPP_HOCON_EXPORT std::shared_ptr<config_document> parse_file(std::string input_file_path,
                                                                    config_parse_options options);

    /** Parses a file into a config_document instance using default options. */
    LIBCPP_HOCON_EXPORT std::shared_ptr<config_document> parse_file(std::string input_file_path);

    /**
     * Parses a string which should be valid HOCON or JSON.
     *
     * @param s string to parse
     * @param options parse options
     * @return the parsed configuration
     */
    LIBCPP_HOCON_EXPORT std::shared_ptr<config_document> parse_string(std::string s, config_parse_options options);

    /** Parses a string into a config_document instance using default options. */
    LIBCPP_HOCON_EXPORT std::shared_ptr<config_document> parse_string(std::string s);

}}  // namespace hocon::config_document_factory
