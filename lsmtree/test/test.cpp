#include <lsmtree.hpp>
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <filesystem>

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
    std::vector<char> buffer1(mt.GetBuffer(), mt.GetBuffer() + 32);
    std::vector<char> expect1{0x01, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x04, 0x00, 0x00, 0x00, 'a', 'b', 'c','d'};
    REQUIRE(buffer1 == expect1);

    REQUIRE(true == mt.CheckSpaceEnough("789"));
    mt.AddItem("789");
    std::vector<char> buffer2(mt.GetBuffer(), mt.GetBuffer() + 32);
    std::vector<char> expect2{0x02, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
                             0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x03, 0x00, 0x00, 0x00, '7', '8', '9',
                             0x04, 0x00, 0x00, 0x00, 'a', 'b', 'c','d'};
    REQUIRE(buffer2 == expect2);

    REQUIRE(false == mt.CheckSpaceEnough("x"));
}

TEST_CASE("test_engine_function")
{
    namespace fs = std::filesystem;
    fs::remove_all("/tmp/test_lsmtree");
    fs::create_directory("/tmp/test_lsmtree");
    lsmtree::Engine engine("/tmp/test_lsmtree", 4096);
    engine.Set("key", "value");
    REQUIRE("value" == engine.Get("key"));
    REQUIRE("" == engine.Get("test"));
    engine.Set("test", "123456");
    REQUIRE("123456" == engine.Get("test"));
}

TEST_CASE("test_engine_storage")
{
    namespace fs = std::filesystem;
    fs::remove_all("/tmp/test_lsmtree");
    fs::create_directory("/tmp/test_lsmtree");
    lsmtree::Engine engine("/tmp/test_lsmtree", 32);
    engine.Set("key1", "1234567890");
    engine.Set("key2", "X");
    engine.Set("key3", "1234567890");

    REQUIRE(true == fs::exists("/tmp/test_lsmtree/engine.meta"));
    int32_t numSegments;
    FILE *pFile = fopen("/tmp/test_lsmtree/engine.meta", "r");
    fread(&numSegments, sizeof(int32_t), 1, pFile);
    REQUIRE(1 == numSegments);
    fclose(pFile);

    REQUIRE(true == fs::exists("/tmp/test_lsmtree/0.data"));
    std::vector<char> buffer;
    buffer.resize(32);
    pFile = fopen("/tmp/test_lsmtree/0.data", "r");
    fread(buffer.data(), sizeof(char), 32, pFile);
    std::vector<char> expect1{0x02, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
                             0x0d, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                             0x00, 'X', 0x0a, 0x00, 0x00, 0x00, '1', '2',
                             '3', '4', '5', '6', '7', '8', '9','0'};
    REQUIRE(expect1 == buffer);
}
