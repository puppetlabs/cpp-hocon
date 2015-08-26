#pragma once

enum class config_syntax {
    /**
     * Pedantically strict <a href="http://json.org">JSON</a> format; no
     * comments, no unexpected commas, no duplicate keys in the same object.
     * Associated with the <code>.json</code> file extension and
     * <code>application/json</code> Content-Type.
     */
    JSON,

    /**
     * The JSON-superset <a
     * href="https://github.com/typesafehub/config/blob/master/HOCON.md"
     * >HOCON</a> format. Associated with the <code>.conf</code> file extension
     * and <code>application/hocon</code> Content-Type.
     */
    CONF,

    /** Used as a default */
    UNSPECIFIED

    // Original project also supported Java's .properties formats, but we do not
};
