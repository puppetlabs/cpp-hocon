#include <catch.hpp>
#include "test_utils.hpp"

#include <hocon/program_options.hpp>

using namespace std;
using namespace hocon;
using namespace hocon::program_options;

TEST_CASE("converts hocon obj to boost::po") {
    po::options_description opts("");
    opts.add_options()("foo", "description");
    po::variables_map vm;
    po::store(parse_string(string("{foo:b}"), opts), vm);
    REQUIRE(vm.count("foo") == 1);
}

TEST_CASE("converts structured hocon to boost::po") {
    po::options_description opts("");
    opts.add_options()("foo.bar", "description");
    po::variables_map vm;
    po::store(parse_string(string("{foo : { bar : baz }}"), opts), vm);
    REQUIRE(vm.count("foo.bar") == 1);
}

TEST_CASE("arrays with only values are converted to boost::po") {
    po::options_description opts("");
    opts.add_options()("foo", po::value<vector<string>>(), "description");
    po::variables_map vm;
    po::store(parse_string(string("{foo : [ bar, baz ]}"), opts), vm);
    REQUIRE(vm.count("foo") == 1);
}

TEST_CASE("unregistered keys cause an exception when `allow_unregistered` is false") {
    po::options_description opts("");
    opts.add_options()("foo", "description");
    po::variables_map vm;
    REQUIRE_THROWS(po::store(parse_string(string("{foo : baz, bar : quux}"), opts, false), vm));
}

TEST_CASE("unregistered keys are ignored when `allow_unregistered` is true") {
    po::options_description opts("");
    opts.add_options()("foo", "description");
    po::variables_map vm;
    po::store(parse_string(string("{foo : baz, bar : quux}"), opts, true), vm);
    REQUIRE(vm.count("foo") == 1);
    REQUIRE(vm.count("bar") == 0);
}

TEST_CASE("arrays with objects cause an exception in boost::po") {
    po::options_description opts("");
    opts.add_options()("foo", "description");
    po::variables_map vm;
    REQUIRE_THROWS(po::store(parse_string(string("{foo : [ { bar : baz } ]}"), opts), vm));
}

TEST_CASE("Arrays mixed with objects cause an exception in boost:po") {
    po::options_description opts("");
    opts.add_options()
         ("foo", po::value<vector<string>>(), "description");
    po::variables_map vm;
    REQUIRE_THROWS(po::store(parse_string(string("{foo : [ quux, {bar : baz } ] }"), opts), vm));
}
