//#include <catch.hpp>
//#include "test_utils.hpp"
//
//#include <hocon/config.hpp>
//#include <internal/values/simple_config_object.hpp>
//
//#include <internal/resolve_result.hpp>
//#include <hocon/config_resolve_options.hpp>
//#include <internal/resolve_context.hpp>
//
//using namespace std;
//using namespace hocon;
//
//static auto const simple_object = parse_object(R"(
//{
//    "foo" : 42,
//    "bar" : {
//        "int" : 43,
//        "bool" : true,
//        "null" : null,
//        "string" : "hello",
//        "double" : 3.14
//    }
//}
//)");
//
//static shared_value resolve_without_fallbacks (shared_value s, shared_object root) {
//    auto options = config_resolve_options(false);
//    return resolve_context::resolve(s, root, options);
//}

//TEST_CASE("resolve trivial key") {
//    auto s = subst("foo");
//    auto v = resolve_without_fallbacks(s, simple_object);
//    REQUIRE(int_value(42) == v);
//}
