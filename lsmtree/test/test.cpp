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

TEST_CASE("test_memtable")
{
    lsmtree::MemTable mt(32);
    REQUIRE(true == mt.CheckSpaceEnough("abcd"));
    mt.AddItem("abcd");
    std::vector<char> buffer1(mt.buffer_, mt.buffer_ + 32);
    std::vector<char> expect1{0x01, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x04, 0x00, 0x00, 0x00, 'a', 'b', 'c','d'};
    REQUIRE(buffer1 == expect1);

    REQUIRE(true == mt.CheckSpaceEnough("789"));
    mt.AddItem("789");
    std::vector<char> buffer2(mt.buffer_, mt.buffer_ + 32);
    std::vector<char> expect2{0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
                             0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x03, 0x00, 0x00, 0x00, '7', '8', '9',
                             0x04, 0x00, 0x00, 0x00, 'a', 'b', 'c','d'};
    REQUIRE(buffer2 == expect2);

    REQUIRE(false == mt.CheckSpaceEnough("x"));
}
