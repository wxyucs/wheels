#include <filesystem>
#include <limits>
#include <benchmark/benchmark.h>
#include <lsmtree.hpp>

/*
static void BM_expensive(benchmark::State& state) // NOLINT google-runtime-references
{
    if (state.thread_index == 0)
    {
        // Setup code here.
    }
    while (state.KeepRunning())
    {
        std::string value = lsmtree::expensive(static_cast<std::size_t>(state.range(0)));
        benchmark::DoNotOptimize(value.data());
        benchmark::ClobberMemory();
    }
    if (state.thread_index == 0)
    {
        // Teardown code here.
    }
}

static void BM_exclaim(benchmark::State& state) // NOLINT google-runtime-references
{
    while (state.KeepRunning())
    {
        std::string value = lsmtree::exclaim("hello");
        benchmark::DoNotOptimize(value.data());
        benchmark::ClobberMemory();
    }
}
*/

static void BM_engine(benchmark::State& state)
{
    using namespace std;
    using namespace benchmark;
    using namespace lsmtree;
    Engine *engine = nullptr;
    size_t repeat = size_t(state.range(0));
    vector<string> keys;
    vector<string> values;
    if (state.thread_index == 0)
    {
        // Setup code here.
        const std::string testdir = "/tmp/benchmark_lsmtree";
        filesystem::remove_all(testdir);
        filesystem::create_directory(testdir);
        engine = new Engine(testdir, 4ll * 1024 *1024);
        keys.resize(repeat);
        values.resize(repeat);
        for (size_t i = 0; i < repeat; ++i) {
            keys[i] = to_string(numeric_limits<size_t>::max() - i).substr(0, 20);
            values[i] = keys[i] + keys[i] + keys[i] + keys[i] + keys[i];
        }
    }
    while (state.KeepRunning()) {
        for (size_t i = 0; i < repeat; ++i) {
            engine->Set(keys[i], values[i]);
        }
    }
    if (state.thread_index == 0)
    {
        // Teardown code here.
        delete engine;
        state.counters["insert_items_per_seconds"] =
            Counter(double(repeat), Counter::kIsRate);
        // state.SetItemsProcessed(int64_t(repeat));
    }
}

auto main(int argc, char* argv[]) -> int
{
    // benchmark::RegisterBenchmark("BM_exclaim", BM_exclaim)->Threads(2)->Threads(4)->Threads(8);                   // NOLINT clang-analyzer-cplusplus.NewDeleteLeaks
    // benchmark::RegisterBenchmark("BM_expensive", BM_expensive)->Threads(2)->Threads(4)->Threads(8)->Arg(1000000); // NOLINT clang-analyzer-cplusplus.NewDeleteLeaks
    benchmark::RegisterBenchmark("BM_engine", BM_engine)->
        Unit(benchmark::kMillisecond)->
        Arg(100000); // NOLINT clang-analyzer-cplusplus.NewDeleteLeaks

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}
