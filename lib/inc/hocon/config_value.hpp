#pragma once

#include "config_origin.hpp"
#include "config_render_options.hpp"
#include "config_mergeable.hpp"
#include "path.hpp"
#include <string>
#include <vector>
#include "export.h"

namespace hocon {

    class unmergeable;
    class resolve_source;
    class resolve_context;

    template<typename T>
    struct resolve_result;

    enum class resolve_status { RESOLVED, UNRESOLVED };

    /**
     * An immutable value, following the <a href="http://json.org">JSON</a> type
     * schema.
     *
     * <p>
     * Because this object is immutable, it is safe to use from multiple threads and
     * there's no need for "defensive copies."
     *
     * <p>
     * <em>Do not implement interface {@code ConfigValue}</em>; it should only be
     * implemented by the config library. Arbitrary implementations will not work
     * because the library internals assume a specific concrete implementation.
     * Also, this interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class LIBCPP_HOCON_EXPORT config_value : public config_mergeable, public std::enable_shared_from_this<config_value> {
        friend class token;
        friend class value;
        friend class default_transformer;
        friend class config;
        friend class config_object;
        friend class simple_config_object;
        friend class simple_config_list;
        friend class config_concatenation;
        friend class resolve_context;
        friend class config_delayed_merge;
        friend class config_delayed_merge_object;
    public:
        /**
         * The type of a configuration value (following the <a
         * href="http://json.org">JSON</a> type schema).
         */
        enum class type {
            OBJECT, LIST, NUMBER, BOOLEAN, CONFIG_NULL, STRING, UNSPECIFIED
        };
        static char const* type_name(type t) {
            switch (t) {
                case type::OBJECT: return "object";
                case type::LIST: return "list";
                case type::NUMBER: return "number";
                case type::BOOLEAN: return "boolean";
                case type::CONFIG_NULL: return "null";
                case type::STRING: return "string";
                case type::UNSPECIFIED: return "unspecified";
                default: throw std::logic_error("Got impossible value for type enum");
            }
        }

        /**
         * The origin of the value (file, line number, etc.), for debugging and
         * error messages.
         *
         * @return where the value came from
         */
        virtual shared_origin const& origin() const;

        /**
         * The type of the value; matches the JSON type schema.
         *
         * @return value's type
         */
        virtual type value_type() const = 0;

        /**
         * The printable name of the value type.
         *
         * @return value's type's name
         */
        char const* value_type_name() const {
            return type_name(value_type());
        }

        virtual unwrapped_value unwrapped() const = 0;

        /**
         * Renders the config value as a HOCON string. This method is primarily
         * intended for debugging, so it tries to add helpful comments and
         * whitespace.
         *
         * <p>
         * If the config value has not been resolved (see {@link config#resolve}),
         * it's possible that it can't be rendered as valid HOCON. In that case the
         * rendering should still be useful for debugging but you might not be able
         * to parse it. If the value has been resolved, it will always be parseable.
         *
         * <p>
         * This method is equivalent to
         * {@code render(config_render_options())}.
         *
         * @return the rendered value
         */
        virtual std::string render() const;

        /**
         * Renders the config value to a string, using the provided options.
         *
         * <p>
         * If the config value has not been resolved (see {@link config#resolve}),
         * it's possible that it can't be rendered as valid HOCON. In that case the
         * rendering should still be useful for debugging but you might not be able
         * to parse it. If the value has been resolved, it will always be parseable.
         *
         * <p>
         * If the config value has been resolved and the options disable all
         * HOCON-specific features (such as comments), the rendering will be valid
         * JSON. If you enable HOCON-only features such as comments, the rendering
         * will not be valid JSON.
         *
         * @param options
         *            the rendering options
         * @return the rendered value
         */
        virtual std::string render(config_render_options options) const;

        /**
         * Places the value inside a {@link config} at the given key. See also
         * {@link config_value#at_path(string)}.
         *
         * @param key
         *            key to store this value at.
         * @return a {@code config} instance containing this value at the given key.
         */
        shared_config at_key(std::string const& key) const;

        /**
         * Places the value inside a {@link config} at the given path. See also
         * {@link config_value#at_key(String)}.
         *
         * @param path
         *            path to store this value at.
         * @return a {@code config} instance containing this value at the given
         *         path.
         */
        shared_config at_path(std::string const& path_expression) const;

        /**
         * Returns a {@code config_value} based on this one, but with the given
         * origin. This is useful when you are parsing a new format of file or setting
         * comments for a single config_value.
         *
         * @param origin the origin set on the returned value
         * @return the new config_value with the given origin
         */
        virtual shared_value with_origin(shared_origin origin) const;

        /**
         * This is used when including one file in another; the included file is
         * relativized to the path it's included into in the parent file. The point
         * is that if you include a file at foo.bar in the parent, and the included
         * file as a substitution ${a.b.c}, the included substitution now needs to
         * be ${foo.bar.a.b.c} because we resolve substitutions globally only after
         * parsing everything.
         *
         * @param prefix
         * @return value relativized to the given path or the same value if nothing
         *         to do
         */
        virtual shared_value relativized(std::string prefix) const { return shared_from_this(); }

        virtual resolve_status get_resolve_status() const;

        friend resolve_status resolve_status_from_values(std::vector<shared_value> const& v);

        std::shared_ptr<const config_mergeable> with_fallback(std::shared_ptr<const config_mergeable> other) const override;

        virtual bool operator==(config_value const& other) const = 0;

        virtual std::string transform_to_string() const;

    protected:
        config_value(shared_origin origin);

        virtual void render(std::string& result, int indent, bool at_root, std::string const& at_key, config_render_options options) const;
        virtual void render(std::string& result, int indent, bool at_root, config_render_options options) const;
        static void indent(std::string& result, int indent, config_render_options const& options);

        shared_config at_key(shared_origin origin, std::string const& key) const;
        shared_config at_path(shared_origin origin, path raw_path) const;

        virtual shared_value new_copy(shared_origin origin) const = 0;

        virtual resolve_result<shared_value>
            resolve_substitutions(resolve_context const& context, resolve_source const& source) const;

        static std::vector<shared_value> replace_child_in_list(std::vector<shared_value> const& values,
                                                               shared_value const& child, shared_value replacement);
        static bool has_descendant_in_list(std::vector<shared_value> const& values, shared_value const& descendant);

        class modifier {
         public:
            virtual shared_value modify_child_may_throw(std::string key_or_null, shared_value v) = 0;
        };

        class no_exceptions_modifier : public modifier {
        public:
            no_exceptions_modifier(std::string prefix);

            shared_value modify_child_may_throw(std::string key_or_null, shared_value v) override;
            shared_value modify_child(std::string key, shared_value v) const;
        private:
            std::string _prefix;
        };
        void require_not_ignoring_fallbacks() const;

        /* this is virtualized rather than a field because only some subclasses
         * really need to store the boolean, and they may be able to pack it
         * with another boolean to save space.
         */
        virtual bool ignores_fallbacks() const;
        virtual shared_value with_fallbacks_ignored() const;

        shared_value merged_with_the_unmergeable(std::vector<shared_value> stack,
                                                 std::shared_ptr<const unmergeable> fallback) const;
        shared_value merged_with_the_unmergeable(std::shared_ptr<const unmergeable> fallback) const;

        shared_value merged_with_object(std::vector<shared_value> stack, shared_object fallback) const;
        virtual shared_value merged_with_object(shared_object fallback) const;

        shared_value merged_with_non_object(std::vector<shared_value> stack, shared_value fallback) const;
        shared_value merged_with_non_object(shared_value fallback) const;

        virtual shared_value construct_delayed_merge(shared_origin origin, std::vector<shared_value> stack) const;

        shared_value to_fallback_value() const override;

        template<typename T>
        static bool equals(config_value const& other, std::function<bool(T const&)> checker)
        {
            // Use pointer casts to avoid exceptions.
            auto other_t = dynamic_cast<T const*>(&other);
            if (!other_t) {
                return false;
            }

            // Pass to checker as a reference to clarify the object will exist.
            return checker(*other_t);
        }

    private:
        shared_value delay_merge(std::vector<shared_value> stack, shared_value fallback) const;

        shared_origin _origin;
    };
}  // namespace hocon
