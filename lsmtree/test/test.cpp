#include <lsmtree.hpp>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

TEST_CASE("test version")
{
    REQUIRE(LSMTREE_VERSION_STRING == std::string("1.0.0"));
}

TEST_CASE("test_exclaim")
{
    std::string value = lsmtree::exclaim("hello");
    REQUIRE(value == std::string("hello!"));
}

TEST_CASE("test_expensive")
{
    std::size_t work = 100;
    std::string result = lsmtree::expensive(work);
    REQUIRE(result == std::string("Expensive work is finished"));
}
