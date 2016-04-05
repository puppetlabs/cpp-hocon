#include <catch.hpp>
#include <hocon/path.hpp>
#include <internal/path_parser.hpp>
#include "test_utils.hpp"

using namespace hocon;
using namespace std;
using namespace hocon::test_utils;

TEST_CASE("path equality", "[path]") {
    SECTION("new_key creates a path with one element") {
        path a = path::new_key("foo.bar");
        REQUIRE(test_path({"foo.bar"}) == a);
        path b = path::new_key("hello");
        REQUIRE(a != b);
    }

    SECTION("new_path correctly splits elements") {
        path a = path::new_path("foo.bar");
        REQUIRE(test_path({ "foo", "bar" }) == a);
        REQUIRE(a != path::new_key("foo.bar"));
    }
}

TEST_CASE("path to string", "[path]") {
    REQUIRE("Path(foo)" == test_path({ "foo" }).to_string());
    REQUIRE("Path(foo.bar)" == test_path({ "foo", "bar" }).to_string());
    REQUIRE("Path(foo.\"bar*\")" == test_path({ "foo", "bar*" }).to_string());
    REQUIRE("Path(\"foo.bar\")" == test_path({ "foo.bar" }).to_string());
}

void render_test(path p, string expected) {
    REQUIRE(p.render() == expected);
    REQUIRE(p == path_parser::parse_path(expected));
    REQUIRE(p == path_parser::parse_path(p.render()));
}

TEST_CASE("path render", "[path]") {
    SECTION("one element") {
        render_test(test_path({"foo"}), "foo");
    }
    SECTION("two elements") {
        render_test(test_path({"foo", "bar"}), "foo.bar");
    }
    SECTION("non-safe character in an element") {
        render_test(test_path({"foo", "bar*"}), "foo.\"bar*\"");
    }
    SECTION("period in an element") {
        render_test(test_path({"foo.bar"}), "\"foo.bar\"");
    }
    SECTION("hyphen in an element") {
        render_test(test_path({"foo-bar"}), "foo-bar");
    }
    SECTION("underscore in an element") {
        render_test(test_path({"foo_bar"}), "foo_bar");
    }
    SECTION("starts with a hyphen") {
        render_test(test_path({"-foo"}), "-foo");
    }
    SECTION("starts with a number") {
        render_test(test_path({"10foo"}), "10foo");
    }
    SECTION("empty elements") {
        render_test(test_path({"", ""}), "\"\".\"\"");
    }
    SECTION("internal space") {
        render_test(test_path({"foo bar"}), "\"foo bar\"");
    }
    SECTION("leading and trailing spaces") {
        render_test(test_path({" foo "}), "\" foo \"");
    }
    SECTION("trailing space only") {
        render_test(test_path({"foo "}), "\"foo \"");
    }
    SECTION("numbers with decimal points") {
        render_test(test_path({"1", "2"}), "1.2");
        render_test(test_path({"1", "2", "3", "4"}), "1.2.3.4");
    }
}

TEST_CASE("path from list", "[path]") {
    REQUIRE(test_path({"foo"}) == path(vector<path> { test_path({"foo"}) }));
    vector<path> paths { test_path({"foo", "bar"}), test_path({"baz", "bat" }) };
    REQUIRE(test_path({"foo", "bar", "baz", "bat"}) == path(paths));
}

TEST_CASE("prepend a path", "[path]") {
    REQUIRE(test_path({"foo", "bar"}) == test_path({"bar"}).prepend(test_path({"foo"})));
    path first = test_path({"a", "b"});
    path second = test_path({"c", "d"});
    REQUIRE(test_path({"a", "b", "c", "d"}) == second.prepend(first));
}

TEST_CASE("path length", "[path]") {
    REQUIRE(test_path({"foo"}).length() == 1);
    REQUIRE(test_path({"foo", "bar", "baz"}).length() == 3);
}

TEST_CASE("path parent", "[path]") {
    REQUIRE(path { } == test_path({"a"}).parent());
    REQUIRE(test_path({"a"}) == test_path({"a", "b"}).parent());
    REQUIRE(test_path({"a", "b"}) == test_path({"a", "b", "c"}).parent());
}

TEST_CASE("path last", "[path]") {
    path empty = path { };
    REQUIRE(empty.last() == nullptr);
    REQUIRE("a" == *test_path({"a"}).last());
    REQUIRE("b" == *test_path({"a", "b"}).last());
}

TEST_CASE("path starts with", "[path]") {
    path full = test_path({"a", "b", "c", "d"});
    REQUIRE(full.starts_with(test_path({"a"})));
    REQUIRE(full.starts_with(test_path({"a", "b", "c"})));
    REQUIRE_FALSE(full.starts_with(test_path({"c", "d"})));
    REQUIRE_FALSE(full.starts_with(test_path({"not_in_full"})));
}

TEST_CASE("invalid paths throw excpetions") {
    REQUIRE_THROWS(path::new_path(""));
    REQUIRE_THROWS(path::new_path(".."));
}

TEST_CASE("can detect weird characters", "[path]") {
    REQUIRE_FALSE(path::has_funky_chars(""));
    REQUIRE(path::has_funky_chars("foo*"));
    REQUIRE_FALSE(path::has_funky_chars("foo"));
}
