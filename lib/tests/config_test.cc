#include <catch.hpp>

#include <hocon/config.hpp>
#include "fixtures.hpp"
#include "test_utils.hpp"

using namespace std;
using namespace hocon;
using namespace hocon::test_utils;

TEST_CASE("should be able to get bare C++ types from a config", "[config]") {

    SECTION("get single values") {
        auto conf = config::parse_file_any_syntax(TEST_FILE_DIR + string("/fixtures/test01.conf"))->resolve();

        REQUIRE(42 == conf->get_int("ints.fortyTwo"));
        REQUIRE(42 == conf->get_long("ints.fortyTwoAgain"));
        REQUIRE(42.1 == conf->get_double("floats.fortyTwoPointOne"));
        REQUIRE(0.33 == conf->get_double("floats.pointThirtyThree"));
        REQUIRE("abcd" == conf->get_string("strings.abcd"));
        REQUIRE("null bar 42 baz true 3.14 hi" == conf->get_string("strings.concatenated"));
        REQUIRE(true == conf->get_bool("booleans.trueAgain"));
        REQUIRE(false == conf->get_bool("booleans.falseAgain"));

        REQUIRE(nullptr == conf->root()->get("not_a_setting"));
    }

    SECTION("get list values") {
        auto conf = config::parse_file_any_syntax(TEST_FILE_DIR + string("/fixtures/test01.conf"))->resolve();

        vector<int> ints { 1, 2, 3 };
        REQUIRE(ints == conf->get_int_list("arrays.ofInt"));

        vector<int64_t> longs { 1, 2, 3 };
        REQUIRE(longs == conf->get_long_list("arrays.ofInt"));

        vector<string> strings { "a", "b", "c" };
        REQUIRE(strings == conf->get_string_list("arrays.ofString"));

        vector<double> doubles { 3.14, 4.14, 5.14 };
        REQUIRE(doubles == conf->get_double_list("arrays.ofDouble"));

        vector<bool> bools { true, false };
        REQUIRE(bools == conf->get_bool_list("arrays.ofBoolean"));

        REQUIRE(3 == conf->get_object_list("arrays.ofObject").size());

        auto bad_list = "bad : [ 1, \"a string\", 4.5 ]";
        auto string_conf = config::parse_string(bad_list);
        REQUIRE_THROWS(string_conf->get_int_list("bad"));
    };
}

TEST_CASE("correct exceptions should be thrown", "[config]") {
    SECTION("missing exception should be thrown when the value is not in the config") {
        bool thrown = false;
        try {
            auto conf = config::parse_file_any_syntax(TEST_FILE_DIR + string("/fixtures/test01.conf"))->resolve();
            conf->get_int("badSetting");
        } catch (const config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "No configuration setting found");
            REQUIRE_STRING_CONTAINS(e.what(), "badSetting");
        }
        REQUIRE(thrown);
    }

    SECTION("a missing exception should be thrown when single-quoted strings are queried incorrectly") {
        bool thrown = false;
        try {
            auto conf = config::parse_string("object : { 'key' : value }");
            conf->get_string("key");
        } catch (const config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "No configuration setting found");
            REQUIRE_STRING_CONTAINS(e.what(), "'key'");
        }
        REQUIRE(thrown);
    }

    SECTION("null exception should be thrown when a null value is queried with a different type") {
        bool thrown = false;
        try {
            auto conf = config::parse_string("object : null");
            conf->get_int("object");
        } catch (const config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "Configuration key \"object\" is null");
        }
        REQUIRE(thrown);
    }

    SECTION("wrong type exception should be thrown when a value cannot be converted to the queried type") {
        bool thrown = false;
        try {
            auto conf = config::parse_string("object : { key : value }");
            conf->get_string("object");
        } catch (const config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "object could not be converted");
        }
        REQUIRE(thrown);
    }

    SECTION("not resolved error message should display when config has not been resolved") {
        bool thrown = false;
        try {
            auto conf = config::parse_string("a : b\nc : ${a}");
            conf->get_string("c");
        } catch (const config_exception& e) {
            thrown = true;
            REQUIRE_STRING_CONTAINS(e.what(), "c has not been resolved");
        }
        REQUIRE(thrown);
    }
}

TEST_CASE("should correctly parse durations", "[config]") {
    auto conf = config::parse_file_any_syntax(TEST_FILE_DIR + string("/fixtures/test01.conf"))->resolve();

    SECTION("should be able to fetch number nodes as durations", "[config]") {
        REQUIRE(1 == conf->get_duration("durations.secondAsNumber", time_unit::SECONDS));
    }

    SECTION("should be able to get durations in specific units", "[config]") {
        // Get as a long
        REQUIRE(1 == conf->get_duration("durations.second", time_unit::SECONDS));
        REQUIRE(500 == conf->get_duration("durations.halfSecond", time_unit::MILLISECONDS));
        REQUIRE(1 == conf->get_duration("durations.millis", time_unit::MILLISECONDS));
        REQUIRE(1000 == conf->get_duration("durations.second", time_unit::MILLISECONDS));
        REQUIRE(60 == conf->get_duration("durations.minute", time_unit::SECONDS));
        REQUIRE(60 == conf->get_duration("durations.hour", time_unit::MINUTES));
        REQUIRE(24 == conf->get_duration("durations.day", time_unit::HOURS));
        REQUIRE(-4 == conf->get_duration("durations.minusSeconds", time_unit::SECONDS));
        REQUIRE(43 == conf->get_duration("durations.secondWithFractional", time_unit::SECONDS));
        REQUIRE(43200 == conf->get_duration("durations.secondWithFractional", time_unit::MILLISECONDS));
        REQUIRE(9223372036854775807 == conf->get_duration("durations.largeNanos", time_unit::NANOSECONDS));
        REQUIRE(-9223372036854775807 == conf->get_duration("durations.minusLargeNanos", time_unit::NANOSECONDS));
        // getting as a long truncates when casting to a larger value
        REQUIRE(0 == conf->get_duration("durations.minute", time_unit::HOURS));
        REQUIRE(9223372036 == conf->get_duration("durations.largeNanos", time_unit::SECONDS));
        REQUIRE(153722867 == conf->get_duration("durations.largeNanos", time_unit::MINUTES));
        REQUIRE(2562047 == conf->get_duration("durations.largeNanos", time_unit::HOURS));
        REQUIRE(106751 == conf->get_duration("durations.largeNanos", time_unit::DAYS));
    }

    SECTION("should throw an exception when overflow occurs", "[config]") {
        REQUIRE_THROWS(conf->get_duration("durations.largeDays", time_unit::DAYS));
        REQUIRE_THROWS(conf->get_duration("durations.largeDays", time_unit::NANOSECONDS));
    }
}
