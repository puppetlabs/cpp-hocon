#include <catch.hpp>

#include <hocon/config.hpp>
#include "fixtures.hpp"

using namespace std;
using namespace hocon;

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

        vector<int> ints { 1,2,3 };
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