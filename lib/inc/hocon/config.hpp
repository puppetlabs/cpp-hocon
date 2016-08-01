#pragma once

#include "types.hpp"
#include "config_mergeable.hpp"
#include "config_origin.hpp"
#include "config_object.hpp"
#include "config_resolve_options.hpp"
#include "config_value.hpp"
#include "config_list.hpp"
#include "config_exception.hpp"
#include "path.hpp"
#include <vector>
#include <string>
#include <set>
#include "export.h"

namespace hocon {

    /**
     * An immutable map from config paths to config values. Paths are dot-separated
     * expressions such as <code>foo.bar.baz</code>. Values are as in JSON
     * (booleans, strings, numbers, lists, or objects), represented by
     * {@link config_value} instances. Values accessed through the
     * <code>config</code> interface are never null.
     *
     * <p>
     * {@code config} is an immutable object and thus safe to use from multiple
     * threads. There's never a need for "defensive copies."
     *
     * <p>
     * Fundamental operations on a {@code config} include getting configuration
     * values, <em>resolving</em> substitutions with {@link config#resolve()}, and
     * merging configs using {@link config#with_fallback(config_mergeable)}.
     *
     * <p>
     * All operations return a new immutable {@code config} rather than modifying
     * the original instance.
     *
     * <p>
     * <strong>Examples</strong>
     *
     * <p>
     * You can find an example app and library <a
     * href="https://github.com/typesafehub/config/tree/master/examples">on
     * GitHub</a>. Also be sure to read the <a
     * href="package-summary.html#package_description">package overview</a> which
     * describes the big picture as shown in those examples.
     *
     * <p>
     * <strong>Paths, keys, and config vs. config_object</strong>
     *
     * <p>
     * <code>config</code> is a view onto a tree of {@link config_object}; the
     * corresponding object tree can be found through {@link config#root()}.
     * <code>config_object</code> is a map from config <em>keys</em>, rather than
     * paths, to config values. Think of <code>config_object</code> as a JSON object
     * and <code>config</code> as a configuration API.
     *
     * <p>
     * The API tries to consistently use the terms "key" and "path." A key is a key
     * in a JSON object; it's just a string that's the key in a map. A "path" is a
     * parseable expression with a syntax and it refers to a series of keys. Path
     * expressions are described in the <a
     * href="https://github.com/typesafehub/config/blob/master/HOCON.md">spec for
     * Human-Optimized Config Object Notation</a>. In brief, a path is
     * period-separated so "a.b.c" looks for key c in object b in object a in the
     * root object. Sometimes double quotes are needed around special characters in
     * path expressions.
     *
     * <p>
     * The API for a {@code config} is in terms of path expressions, while the API
     * for a {@code config_object} is in terms of keys. Conceptually, {@code config}
     * is a one-level map from <em>paths</em> to values, while a
     * {@code config_object} is a tree of nested maps from <em>keys</em> to values.
     *
     * <p>
     * Use {@link config_util#join_path} and {@link config_util#split_path} to convert
     * between path expressions and individual path elements (keys).
     *
     * <p>
     * Another difference between {@code config} and {@code config_object} is that
     * conceptually, {@code config_value}s with a {@link config_value#value_type()
     * value_type()} of {@link config_value::type#NULL NULL} exist in a
     * {@code config_object}, while a {@code config} treats null values as if they
     * were missing. (With the exception of two methods: {@link config#has_path_or_null}
     * and {@link config#get_is_null} let you detect <code>null</code> values.)
     *
     * <p>
     * <strong>Getting configuration values</strong>
     *
     * <p>
     * The "getters" on a {@code config} all work in the same way. They never return
     * null, nor do they return a {@code config_value} with
     * {@link config_value#value_type() value_type()} of {@link config_value::type#NULL
     * NULL}. Instead, they throw {@link config_exception} if the value is
     * completely absent or set to null. If the value is set to null, a subtype of
     * {@code config_exception.missing} called {@link config_exception.null} will be
     * thrown. {@link config_excpetion.wrong_type} will be thrown anytime you ask for
     * a type and the value has an incompatible type. Reasonable type conversions
     * are performed for you though.
     *
     * <p>
     * <strong>Iteration</strong>
     *
     * <p>
     * If you want to iterate over the contents of a {@code config}, you can get its
     * {@code config_object} with {@link #root()}, and then iterate over the
     * {@code config_object} (which implements <code>java.util.Map</code>). Or, you
     * can use {@link #entry_set()} which recurses the object tree for you and builds
     * up a <code>set</code> of all path-value pairs where the value is not null.
     *
     * <p>
     * <strong>Resolving substitutions</strong>
     *
     * <p>
     * <em>Substitutions</em> are the <code>${foo.bar}</code> syntax in config
     * files, described in the <a href=
     * "https://github.com/typesafehub/config/blob/master/HOCON.md#substitutions"
     * >specification</a>. Resolving substitutions replaces these references with real
     * values.
     *
     * <p>
     * Before using a {@code config} it's necessary to call {@link config#resolve()}
     * to handle substitutions (though {@link config_factory#load()} and similar
     * methods will do the resolve for you already).
     *
     * <p>
     * <strong>Merging</strong>
     *
     * <p>
     * The full <code>config</code> for your application can be constructed using
     * the associative operation {@link config#with_fallback(config_mergeable)}. If
     * you use {@link config_factory#load()} (recommended), it merges system
     * properties over the top of <code>application.conf</code> over the top of
     * <code>reference.conf</code>, using <code>with_fallback</code>. You can add in
     * additional sources of configuration in the same way (usually, custom layers
     * should go either just above or just below <code>application.conf</code>,
     * keeping <code>reference.conf</code> at the bottom and system properties at
     * the top).
     *
     * <p>
     * <strong>Serialization</strong>
     *
     * <p>
     * Convert a <code>config</code> to a JSON or HOCON string by calling
     * {@link config_object#render()} on the root object,
     * <code>my_config.root().render()</code>. There's also a variant
     * {@link config_object#render(config_render_options)} which allows you to control
     * the format of the rendered string. (See {@link config_render_options}.) Note
     * that <code>config</code> does not remember the formatting of the original
     * file, so if you load, modify, and re-save a config file, it will be
     * substantially reformatted.
     *
     * <p>
     * As an alternative to {@link config_object#render()}, the
     * <code>to_string()</code> method produces a debug-output-oriented
     * representation (which is not valid JSON).
     *
     * <p>
     * <strong>This is an interface but don't implement it yourself</strong>
     *
     * <p>
     * <em>Do not implement {@code config}</em>; it should only be implemented by
     * the config library. Arbitrary implementations will not work because the
     * library internals assume a specific concrete implementation. Also, this
     * interface is likely to grow new methods over time, so third-party
     * implementations will break.
     */
    class LIBCPP_HOCON_EXPORT config : public config_mergeable, public std::enable_shared_from_this<config> {
        friend class config_object;
        friend class config_value;
        friend class config_parseable;
        friend class parseable;

    public:
        /**
         * Parses a file with a flexible extension. If the <code>fileBasename</code>
         * already ends in a known extension, this method parses it according to
         * that extension (the file's syntax must match its extension). If the
         * <code>fileBasename</code> does not end in an extension, it parses files
         * with all known extensions and merges whatever is found.
         *
         * <p>
         * In the current implementation, the extension ".conf" forces
         * {@link ConfigSyntax#CONF}, ".json" forces {@link ConfigSyntax#JSON}.
         * When merging files, ".conf" falls back to ".json".
         *
         * <p>
         * Future versions of the implementation may add additional syntaxes or
         * additional extensions. However, the ordering (fallback priority) of the
         * three current extensions will remain the same.
         *
         * <p>
         * If <code>options</code> forces a specific syntax, this method only parses
         * files with an extension matching that syntax.
         *
         * <p>
         * If {@link ConfigParseOptions#getAllowMissing options.getAllowMissing()}
         * is true, then no files have to exist; if false, then at least one file
         * has to exist.
         *
         * @param fileBasename
         *            a filename with or without extension
         * @param options
         *            parse options
         * @return the parsed configuration
         */
        static shared_config parse_file_any_syntax(std::string file_basename, config_parse_options options);

        /**
         * Like {@link #parseFileAnySyntax(File,ConfigParseOptions)} but always uses
         * default parse options.
         *
         * @param fileBasename
         *            a filename with or without extension
         * @return the parsed configuration
         */
        static shared_config parse_file_any_syntax(std::string file_basename);

        /**
         * Parses a string (which should be valid HOCON or JSON by default, or
         * the syntax specified in the options otherwise).
         *
         * @param s string to parse
         * @param options parse options
         * @return the parsed configuration
         */
        static shared_config parse_string(std::string s, config_parse_options options);

        /**
         * Parses a string (which should be valid HOCON or JSON).
         *
         * @param s string to parse
         * @return the parsed configuration
         */
        static shared_config parse_string(std::string s);

        /**
         * Gets the {@code Config} as a tree of {@link ConfigObject}. This is a
         * constant-time operation (it is not proportional to the number of values
         * in the {@code Config}).
         *
         * @return the root object in the configuration
         */
        virtual shared_object root() const;

        /**
         * Gets the origin of the {@code Config}, which may be a file, or a file
         * with a line number, or just a descriptive phrase.
         *
         * @return the origin of the {@code Config} for use in error messages
         */
        virtual shared_origin origin() const;

        std::shared_ptr<const config_mergeable> with_fallback(std::shared_ptr<const config_mergeable> other) const override;

        shared_value to_fallback_value() const override;

        /**
         * Returns a replacement config with all substitutions (the
         * <code>${foo.bar}</code> syntax, see <a
         * href="https://github.com/typesafehub/config/blob/master/HOCON.md">the
         * spec</a>) resolved. Substitutions are looked up using this
         * <code>config</code> as the root object, that is, a substitution
         * <code>${foo.bar}</code> will be replaced with the result of
         * <code>get_value("foo.bar")</code>.
         *
         * <p>
         * This method uses {@link config_resolve_options()}, there is
         * another variant {@link config#resolve(config_resolve_options)} which lets
         * you specify non-default options.
         *
         * <p>
         * A given {@link config} must be resolved before using it to retrieve
         * config values, but ideally should be resolved one time for your entire
         * stack of fallbacks (see {@link config#with_fallback}). Otherwise, some
         * substitutions that could have resolved with all fallbacks available may
         * not resolve, which will be potentially confusing for your application's
         * users.
         *
         * <p>
         * <code>resolve()</code> should be invoked on root config objects, rather
         * than on a subtree (a subtree is the result of something like
         * <code>config.get_config("foo")</code>). The problem with
         * <code>resolve()</code> on a subtree is that substitutions are relative to
         * the root of the config and the subtree will have no way to get values
         * from the root. For example, if you did
         * <code>config.get_config("foo").resolve()</code> on the below config file,
         * it would not work:
         *
         * <pre>
         *   common-value = 10
         *   foo {
         *      whatever = ${common-value}
         *   }
         * </pre>
         *
         * <p>
         * Many methods on {@link config_factory} such as
         * {@link config_factory#load()} automatically resolve the loaded
         * <code>config</code> on the loaded stack of config files.
         *
         * <p>
         * Resolving an already-resolved config is a harmless no-op, but again, it
         * is best to resolve an entire stack of fallbacks (such as all your config
         * files combined) rather than resolving each one individually.
         *
         * @return an immutable object with substitutions resolved
         */
        virtual shared_config resolve() const;

        /**
         * Like {@link config#resolve()} but allows you to specify non-default
         * options.
         *
         * @param options
         *            resolve options
         * @return the resolved <code>config</code> (may be only partially resolved if options are
         *          set to allow unresolved)
         */
        virtual shared_config resolve(config_resolve_options options) const;

        /**
         * Checks whether the config is completely resolved. After a successful call
         * to {@link config#resolve()} it will be completely resolved, but after
         * calling {@link config#resolve(config_resolve_options)} with
         * <code>allow_unresolved</code> set in the options, it may or may not be
         * completely resolved. A newly-loaded config may or may not be completely
         * resolved depending on whether there were substitutions present in the
         * file.
         *
         * @return true if there are no unresolved substitutions remaining in this
         *         configuration.
         */
        virtual bool is_resolved() const;

        /**
         * Like {@link config#resolve()} except that substitution values are looked
         * up in the given source, rather than in this instance. This is a
         * special-purpose method which doesn't make sense to use in most cases;
         * it's only needed if you're constructing some sort of app-specific custom
         * approach to configuration. The more usual approach if you have a source
         * of substitution values would be to merge that source into your config
         * stack using {@link config#withFallback} and then resolve.
         * <p>
         * Note that this method does NOT look in this instance for substitution
         * values. If you want to do that, you could either merge this instance into
         * your value source using {@link config#with_fallback}, or you could resolve
         * multiple times with multiple sources (using
         * {@link config_resolve_options#setAllowUnresolved(boolean)} so the partial
         * resolves don't fail).
         *
         * @param source
         *            configuration to pull values from
         * @return an immutable object with substitutions resolved
         */
        virtual shared_config resolve_with(shared_config source) const;

        /**
         * Like {@link config#resolve_with(config)} but allows you to specify
         * non-default options.
         *
         * @param source
         *            source configuration to pull values from
         * @param options
         *            resolve options
         * @return the resolved <code>config</code> (may be only partially resolved
         *         if options are set to allow unresolved)
         */
        virtual shared_config resolve_with(shared_config source, config_resolve_options options) const;

        /**
         * Validates this config against a reference config, throwing an exception
         * if it is invalid. The purpose of this method is to "fail early" with a
         * comprehensive list of problems; in general, anything this method can find
         * would be detected later when trying to use the config, but it's often
         * more user-friendly to fail right away when loading the config.
         *
         * <p>
         * Using this method is always optional, since you can "fail late" instead.
         *
         * <p>
         * You must restrict validation to paths you "own" (those whose meaning are
         * defined by your code module). If you validate globally, you may trigger
         * errors about paths that happen to be in the config but have nothing to do
         * with your module. It's best to allow the modules owning those paths to
         * validate them. Also, if every module validates only its own stuff, there
         * isn't as much redundant work being done.
         *
         * <p>
         * If no paths are specified in <code>check_valid()</code>'s parameter list,
         * validation is for the entire config.
         *
         * <p>
         * If you specify paths that are not in the reference config, those paths
         * are ignored. (There's nothing to validate.)
         *
         * <p>
         * Here's what validation involves:
         *
         * <ul>
         * <li>All paths found in the reference config must be present in this
         * config or an exception will be thrown.
         * <li>
         * Some changes in type from the reference config to this config will cause
         * an exception to be thrown. Not all potential type problems are detected,
         * in particular it's assumed that strings are compatible with everything
         * except objects and lists. This is because string types are often "really"
         * some other type (system properties always start out as strings, or a
         * string like "5ms" could be used with {@link #get_milliseconds}). Also,
         * it's allowed to set any type to null or override null with any type.
         * <li>
         * Any unresolved substitutions in this config will cause a validation
         * failure; both the reference config and this config should be resolved
         * before validation. If the reference config is unresolved, it's a bug in
         * the caller of this method.
         * </ul>
         *
         * <p>
         * If you want to allow a certain setting to have a flexible type (or
         * otherwise want validation to be looser for some settings), you could
         * either remove the problematic setting from the reference config provided
         * to this method, or you could intercept the validation exception and
         * screen out certain problems. Of course, this will only work if all other
         * callers of this method are careful to restrict validation to their own
         * paths, as they should be.
         *
         * <p>
         * If validation fails, the thrown exception contains a list of all problems
         * found. The exception will have all the problem concatenated into one huge string.
         *
         * <p>
         * Again, <code>check_valid()</code> can't guess every domain-specific way a
         * setting can be invalid, so some problems may arise later when attempting
         * to use the config. <code>check_valid()</code> is limited to reporting
         * generic, but common, problems such as missing settings and blatant type
         * incompatibilities.
         *
         * @param reference
         *            a reference configuration
         * @param restrictToPaths
         *            only validate values underneath these paths that your code
         *            module owns and understands
         */
        virtual void check_valid(shared_config reference, std::vector<std::string> restrict_to_paths) const;

        /**
         * Checks whether a value is present and non-null at the given path. This
         * differs in two ways from {@code Map.containsKey()} as implemented by
         * {@link config_object}: it looks for a path expression, not a key; and it
         * returns false for null values, while {@code contains_key()} returns true
         * indicating that the object contains a null value for the key.
         *
         * <p>
         * If a path exists according to {@link #has_path(string)}, then
         * {@link #get_value(string)} will never throw an exception. However, the
         * typed getters will still throw if the value is not convertible to the
         * requested type.
         *
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            the path expression
         * @return true if a non-null value is present at the path
         */
        virtual bool has_path(std::string const& path) const;

        /**
         * Checks whether a value is present at the given path, even
         * if the value is null. Most of the getters on
         * <code>config</code> will throw if you try to get a null
         * value, so if you plan to call {@link #get_value(string)},
         * {@link #get_int(string)}, or another getter you may want to
         * use plain {@link #has_path(string)} rather than this method.
         *
         * <p>
         * To handle all three cases (unset, null, and a non-null value)
         * the code might look like:
         * <pre><code>
         * if (config.has_path_or_null(path)) {
         *     if (config.get_is_null(path)) {
         *        // handle null setting
         *     } else {
         *        // get and use non-null setting
         *     }
         * } else {
         *     // handle entirely unset path
         * }
         * </code></pre>
         *
         * <p> However, the usual thing is to allow entirely unset
         * paths to be a bug that throws an exception (because you set
         * a default in your <code>reference.conf</code>), so in that
         * case it's OK to call {@link #get_is_null(string)} without
         * checking <code>has_path_or_null</code> first.
         *
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            the path expression
         * @return true if a value is present at the path, even if the value is null
         */
        virtual bool has_path_or_null(std::string const& path) const;

        /**
         * Returns true if the {@code Config}'s root object contains no key-value
         * pairs.
         *
         * @return true if the configuration is empty
         */
        virtual bool is_empty() const;

        /**
         * Returns the set of path-value pairs, excluding any null values, found by
         * recursing {@link #root() the root object}. Note that this is very
         * different from <code>root().entry_set()</code> which returns the set of
         * immediate-child keys in the root object and includes null values.
         * <p>
         * Entries contain <em>path expressions</em> meaning there may be quoting
         * and escaping involved. Parse path expressions with
         * {@link config_util#split_path}.
         * <p>
         * Because a <code>config</code> is conceptually a single-level map from
         * paths to values, there will not be any {@link config_object} values in the
         * entries (that is, all entries represent leaf nodes). Use
         * {@link config_object} rather than <code>config</code> if you want a tree.
         * (OK, this is a slight lie: <code>config</code> entries may contain
         * {@link config_list} and the lists may contain objects. But no objects are
         * directly included as entry values.)
         *
         * @return set of paths with non-null values, built up by recursing the
         *         entire tree of {@link config_object} and creating an entry for
         *         each leaf value.
         */
        virtual std::set<std::pair<std::string, std::shared_ptr<const config_value>>> entry_set() const;

        /**
         * Checks whether a value is set to null at the given path,
         * but throws an exception if the value is entirely
         * unset. This method will not throw if {@link
         * #has_path_or_null(string)} returned true for the same path, so
         * to avoid any possible exception check
         * <code>has_path_or_null()</code> first.  However, an exception
         * for unset paths will usually be the right thing (because a
         * <code>reference.conf</code> should exist that has the path
         * set, the path should never be unset unless something is
         * broken).
         *
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            the path expression
         * @return true if the value exists and is null, false if it
         * exists and is not null
         */
        virtual bool get_is_null(std::string const& path) const;

        virtual bool get_bool(std::string const& path) const;
        virtual int get_int(std::string const& path) const;
        virtual int64_t get_long(std::string const& path) const;
        virtual double get_double(std::string const& path) const;
        virtual std::string get_string(std::string const& path) const;
        virtual std::shared_ptr<const config_object> get_object(std::string const& path) const;
        virtual shared_config get_config(std::string const& path) const;
        virtual unwrapped_value get_any_ref(std::string const& path) const;
        virtual std::shared_ptr<const config_value> get_value(std::string const& path) const;

        template<typename T>
        std::vector<T> get_homogeneous_unwrapped_list(std::string const& path) const {
            auto list = boost::get<std::vector<unwrapped_value>>(get_list(path)->unwrapped());
            std::vector<T> T_list;
            for (auto item : list) {
                try {
                    T_list.push_back(boost::get<T>(item));
                } catch (boost::bad_get &ex) {
                    throw config_exception("The list did not contain only the desired type.");
                }
            }
            return T_list;
        }

        virtual shared_list get_list(std::string const& path) const;
        virtual std::vector<bool> get_bool_list(std::string const& path) const;
        virtual std::vector<int> get_int_list(std::string const& path) const;
        virtual std::vector<int64_t> get_long_list(std::string const& path) const;
        virtual std::vector<double> get_double_list(std::string const& path) const;
        virtual std::vector<std::string> get_string_list(std::string const& path) const;
        virtual std::vector<shared_object> get_object_list(std::string const& path) const;
        virtual std::vector<shared_config> get_config_list(std::string const& path) const;

        // TODO: memory and duration parsing

        /**
         * Clone the config with only the given path (and its children) retained;
         * all sibling paths are removed.
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            path to keep
         * @return a copy of the config minus all paths except the one specified
         */
        virtual shared_config with_only_path(std::string const& path) const;

        /**
         * Clone the config with the given path removed.
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            path expression to remove
         * @return a copy of the config minus the specified path
         */
        virtual shared_config without_path(std::string const& path) const;

        /**
         * Places the config inside another {@code config} at the given path.
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            path expression to store this config at.
         * @return a {@code config} instance containing this config at the given
         *         path.
         */
        virtual shared_config at_path(std::string const& path) const;

        /**
         * Places the config inside a {@code config} at the given key. See also
         * at_path(). Note that a key is NOT a path expression (see
         * {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param key
         *            key to store this config at.
         * @return a {@code config} instance containing this config at the given
         *         key.
         */
        virtual shared_config at_key(std::string const& key) const;

        /**
         * Returns a {@code config} based on this one, but with the given path set
         * to the given value. Does not modify this instance (since it's immutable).
         * If the path already has a value, that value is replaced. To remove a
         * value, use withoutPath().
         * <p>
         * Note that path expressions have a syntax and sometimes require quoting
         * (see {@link config_util#join_path} and {@link config_util#split_path}).
         *
         * @param path
         *            path expression for the value's new location
         * @param value
         *            value at the new path
         * @return the new instance with the new map entry
         */
        virtual shared_config with_value(std::string const& path, std::shared_ptr<const config_value> value) const;

        bool operator==(config const& other) const;

        config(shared_object object);

        static shared_object env_variables_as_config_object();

    protected:
        shared_value find(std::string const& path_expression, config_value::type expected) const;
        shared_value find(path path_expression, config_value::type expected, path original_path) const;
        shared_value find(path path_expression, config_value::type expected) const;
        shared_config at_key(shared_origin origin, std::string const& key) const;

        static shared_includer default_includer();

        // TODO: memory and duration parsing

    private:
        shared_value has_path_peek(std::string const& path_expression) const;
        shared_value peek_path(path desired_path) const;

        static void find_paths(std::set<std::pair<std::string, std::shared_ptr<const config_value>>>& entries,
                               path parent, shared_object obj);
        static shared_value throw_if_null(shared_value v, config_value::type expected, path original_path);
        static shared_value find_key(shared_object self, std::string const& key,
                                     config_value::type expected, path original_path);
        static shared_value find_key_or_null(shared_object self, std::string const& key,
                                             config_value::type expected, path original_path);
        static shared_value find_or_null(shared_object self, path desired_path,
                                         config_value::type expected, path original_path);
        shared_value find_or_null(std::string const& path_expression, config_value::type expected) const;
        shared_value find_or_null(path path_expression, config_value::type expected, path original_path) const;

        shared_object _object;
    };

    template<>
    std::vector<int64_t> config::get_homogeneous_unwrapped_list(std::string const& path) const;

}  // namespace hocon
