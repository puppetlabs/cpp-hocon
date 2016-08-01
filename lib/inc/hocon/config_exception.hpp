#pragma once

#include <stdexcept>
#include <string>
#include "config_origin.hpp"

namespace hocon {

    /**
     * All exceptions thrown by the library are subclasses of
     * <code>config_exception</code>.
     */
    struct config_exception : public std::runtime_error {
        config_exception(config_origin const& origin, std::string const& message) :
                runtime_error(origin.description() + ": " + message) { }
        config_exception(std::string const& message) : runtime_error(message) { }

        config_exception(std::string const& message, std::exception const& e) : runtime_error(message + " " + e.what()) { }
    };

    /**
     * Exception indicating that the type of a value does not match the type you
     * requested.
     *
     */
    struct wrong_type_exception : public config_exception {
        wrong_type_exception(config_origin const& origin,
                             std::string const& path, std::string const& expected, std::string const& actual) :
                config_exception(origin, path + " has type " + actual + " rather than " + expected) { }
        using config_exception::config_exception;
    };

    /**
     * Exception indicates that the setting was never set to anything, not even
     * null.
     */
    struct missing_exception : public config_exception {
        missing_exception(std::string const& path) :
                config_exception("No configuration setting found for key '" + path + "'") { }
        using config_exception::config_exception;
    };

    /**
     * Exception indicates that the setting was treated as missing because it
     * was set to null.
     */
    struct null_exception : public missing_exception {
        null_exception(config_origin const& origin, std::string const& path, std::string const& expected = "") :
                missing_exception(origin, (expected.empty() ? "Configuration key \"" + path + "\" is null"
                                                            : "Configuration key \"" + path + "\" is set to null but expected " + expected)) { }
    };

    /**
     * Exception indicating that a value was messed up, for example you may have
     * asked for a duration and the value can't be sensibly parsed as a
     * duration.
     */
    struct bad_value_exception : public config_exception {
        bad_value_exception(config_origin const& origin, std::string const& path, std::string const& message) :
                config_exception(origin, "Invalid value at '" + path + "': " + message) { }
        bad_value_exception(std::string const& path, std::string const& message) :
                config_exception("Invalid value at '" + path + "': " + message) { }
    };

    /**
     * Exception indicating that a path expression was invalid. Try putting
     * double quotes around path elements that contain "special" characters.
     *
     */
    struct bad_path_exception : public config_exception {
        bad_path_exception(config_origin const& origin, std::string const& path, std::string const& message) :
                config_exception(origin, path.empty() ? message : "Invalid path '" + path + "': " + message) { }
        bad_path_exception(std::string const& path, std::string const& message) :
                config_exception(path.empty() ? message : "Invalid path '" + path + "': " + message) { }
    };

    /**
     * Exception indicating that there's a bug in something (possibly the
     * library itself) or the runtime environment is broken. This exception
     * should never be handled; instead, something should be fixed to keep the
     * exception from occurring. This exception can be thrown by any method in
     * the library.
     */
    struct bug_or_broken_exception : public config_exception {
        bug_or_broken_exception(std::string const& message) : config_exception(message) { }
    };

    /**
     * Exception indicating that there was an IO error.
     *
     */
    struct io_exception : public config_exception {
        io_exception(config_origin const& origin, std::string const& message) : config_exception(origin, message) { }
    };

    /**
     * Exception indicating that there was a parse error.
     *
     */
    struct parse_exception : public config_exception {
        parse_exception(config_origin const& origin, std::string const& message) : config_exception(origin, message) { }
    };


    /**
     * Exception indicating that a substitution did not resolve to anything.
     * Thrown by {@link config#resolve}.
     */
    struct unresolved_substitution_exception : public parse_exception {
        unresolved_substitution_exception(config_origin const& origin, std::string const& detail) :
                parse_exception(origin, "Could not resolve subtitution to a value: " + detail) { }
    };

    /**
     * Exception indicating that you tried to use a function that requires
     * substitutions to be resolved, but substitutions have not been resolved
     * (that is, {@link config#resolve} was not called). This is always a bug in
     * either application code or the library; it's wrong to write a handler for
     * this exception because you should be able to fix the code to avoid it by
     * adding calls to {@link config#resolve}.
     */
    struct not_resolved_exception : public bug_or_broken_exception {
        not_resolved_exception(std::string const& message) : bug_or_broken_exception(message) { }
    };

    struct not_possible_to_resolve_exception : public bug_or_broken_exception {
        not_possible_to_resolve_exception(std::string const& message) : bug_or_broken_exception(message) { }
    };

    /**
     * Information about a problem that occurred in {@link config#check_valid}. A
     * {@link validation_failed_exception} thrown from
     * <code>check_valid()</code> includes a list of problems encountered.
     */
    struct validation_problem {
        validation_problem(std::string path_, shared_origin origin_, std::string problem_) :
                path(std::move(path_)), origin(std::move(origin_)), problem(std::move(problem_)) { }

        const std::string path;
        const shared_origin origin;
        const std::string problem;

        std::string to_string() {
            return "ValidationProblem(" + path + "," + origin->description() + "," + problem + ")";
        }
    };

    /**
     * Exception indicating that {@link config#check_valid} found validity
     * problems. The problems are available via the {@link #problems()} method.
     * The <code>get_message()</code> of this exception is a potentially very
     * long string listing all the problems found.
     */
    struct validation_failed_exception : public config_exception {
        validation_failed_exception(std::vector<validation_problem> problems_) :
                config_exception(make_message(problems_)), problems(std::move(problems_)) { }

        const std::vector<validation_problem> problems;

    private:
        static std::string make_message(std::vector<validation_problem> const& problems_) {
            std::string msg;
            for (auto &p : problems_) {
                auto sep = std::string(": ");
                for (auto &s : {p.origin->description(), sep, p.path, sep, p.problem, std::string(", ")}) {
                    msg.append(s);
                }
            }
            if (msg.empty()) {
                throw bug_or_broken_exception("validation_failed_exception must have a non-empty list of problems");
            }
            msg.resize(msg.length() - 2);
            return msg;
        }
    };

    /**
     * Exception that doesn't fall into any other category.
     */
    struct generic_exception : public config_exception {
        generic_exception(std::string const& message) : config_exception(message) { }
    };
}  // namespace hocon
