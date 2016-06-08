#pragma once

#include <hocon/config_origin.hpp>

#include <string>
#include <vector>
#include <memory>

namespace hocon {

    enum class origin_type { GENERIC, FILE, RESOURCE };

    class simple_config_origin : public config_origin, public std::enable_shared_from_this<simple_config_origin> {
    public:
        simple_config_origin(std::string description, int line_number, int end_line_number,
            origin_type org_type, std::string resource_or_null, std::vector<std::string> comments_or_null);

        /** This constructor replaces the new_simple method in the original library. */
        simple_config_origin(std::string description, int line_number = -1, int end_line_number = -1,
                             origin_type org_type = origin_type::GENERIC);

        int line_number() const override;

        std::string const& description() const override;
        std::vector<std::string> const& comments() const override;

        /**
         * Returns a pointer to a copy of this origin with the specified line number
         * as both starting and ending line.
         */
        shared_origin with_line_number(int line_number) const override;

        shared_origin with_comments(std::vector<std::string> comments) const override;
        std::shared_ptr<const simple_config_origin> append_comments(std::vector<std::string> comments) const;
        std::shared_ptr<const simple_config_origin> prepend_comments(std::vector<std::string> comments) const;

        static shared_origin merge_origins(shared_origin a, shared_origin b);
        static shared_origin merge_origins(std::vector<shared_value> const& stack);
        static shared_origin merge_origins(std::vector<shared_origin> const& stack);

        bool operator==(const simple_config_origin &other) const;
        bool operator!=(const simple_config_origin &other) const;

    private:
        static std::shared_ptr<const simple_config_origin> merge_two(std::shared_ptr<const simple_config_origin> a,
                                                                     std::shared_ptr<const simple_config_origin> b);

        // this picks the best pair to merge, because the pair has the most in
        // common. we want to merge two lines in the same file rather than something
        // else with one of the lines; because two lines in the same file can be
        // better consolidated.
        static std::shared_ptr<const simple_config_origin> merge_three(std::shared_ptr<const simple_config_origin> a,
                                                                       std::shared_ptr<const simple_config_origin> b,
                                                                       std::shared_ptr<const simple_config_origin> c);

        static int similarity(std::shared_ptr<const simple_config_origin> a,
                              std::shared_ptr<const simple_config_origin> b);

        std::string _description;
        int _line_number;
        int _end_line_number;
        origin_type _origin_type;
        std::string _resource_or_null;
        std::vector<std::string> _comments_or_null;
    };

}  // namespace hocon
