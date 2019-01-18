#pragma once

#include <string>
#include <stack>
#include <mutex>
#include <unordered_map>
#include <memory>

namespace hocon {

    bool is_whitespace(char codepoint);

    bool is_whitespace_not_newline(char codepoint);

    bool is_C0_control(char c);

    std::string render_json_string(std::string const& s);

    std::string render_string_unquoted_if_possible(std::string const& s);

    void extract_filename_from_path(const std::string& path,
                                    std::string *file_dir,
                                    std::string *file_name); 

    class full_path_operator {
     public:
         full_path_operator():
            _current_dir(""),
            _str_stack() {}

         full_path_operator(const std::string& s):
            _current_dir(s), _str_stack() {}

         void stash();

         void append(const std::string& dir);

         int remove(const std::string& dir);

         std::string extend(const std::string& str);

     private:
         std::string _current_dir;
         std::stack<std::string> _str_stack;
    };

}  // namespace hocon
