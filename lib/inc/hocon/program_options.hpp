#pragma once

#include "config.hpp"
#include "config_list.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace hocon { namespace program_options {
    namespace po = boost::program_options;

    template<class charT>
    po::basic_parsed_options<charT>
    parse_hocon(shared_config cfg, const po::options_description& desc, bool allow_unregistered) {
        po::parsed_options result(&desc);

        std::map<std::string, shared_config> to_do;
        to_do.emplace("", cfg);

        while (!to_do.empty()) {
            auto iter = to_do.begin();
            auto prefix = iter->first;
            auto cfg = iter->second;
            to_do.erase(iter);

            for (const auto& entry : cfg->entry_set()) {
                po::option opt;

                // TODO: enforce unregistered variable flag based on allow_unregistered setting

                if (prefix.empty()) {
                    opt.string_key = entry.first;
                } else {
                    opt.string_key = prefix + "." + entry.first;
                }

                if (entry.second->value_type() == config_value::type::LIST) {
                    // if this is a list, we want to check if any of the entries are
                    // objects. If so, we need to further expand those sub-trees, with
                    // list indices being expanded into our key

                    auto list = std::dynamic_pointer_cast<const config_list>(entry.second);
                    for (size_t i = 0; i < list->size(); ++i) {
                        const auto& value = list->get(i);
                        if (value->value_type() == config_value::type::LIST ||
                            value->value_type() == config_value::type::OBJECT) {
                            boost::throw_exception(po::invalid_config_file_syntax(list->transform_to_string(), po::invalid_syntax::unrecognized_line));
                        } else {
                            opt.value.push_back(value->transform_to_string());
                        }
                    }
                } else {
                    opt.value.push_back(entry.second->transform_to_string());
                }
                if (!opt.value.empty()) {
                    result.options.emplace_back(std::move(opt));
                }
            }
        }

        // let Boost convert our utf-8 entries to wchars if needed
        return po::basic_parsed_options<charT>(result);
    }

    template<class charT>
    po::basic_parsed_options<charT>
    parse_file(std::basic_string<charT> file_basename, const po::options_description& desc, bool allow_unregistered=false) {
        shared_config cfg = config::parse_file_any_syntax(file_basename)->resolve();
        return parse_hocon<charT>(cfg, desc, allow_unregistered);
    }

    template<class charT>
    po::basic_parsed_options<charT>
    parse_string(std::basic_string<charT> s, const po::options_description& desc, bool allow_unregistered=false) {
        shared_config cfg = config::parse_string(s)->resolve();
        return parse_hocon<charT>(cfg, desc, allow_unregistered);
    }
}}
