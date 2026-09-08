#include "BenchmarkRunner.hpp"
#include <compat/Config.hpp>
#include <compat/detail/SelfFormat.hpp>

#if COMPAT_HAS_STD_FORMAT
#  include <format>
#endif

#include <cstdio>
#include <string>
#include <cstdint>

/// <summary>
/// Registers performance benchmarks for string formatting.
/// Compares self-contained FormatToString fallback, native std::format (C++20), and traditional snprintf.
/// </summary>
/// <param name="runner">Benchmark runner instance to register test cases into.</param>
void RegisterFormatBenchmarks(compat::bench::BenchmarkRunner& runner) {
    constexpr uint64_t kDefaultIters = 1000000;

    // =========================================================================
    // 1. Single Argument Formatting (Integer)
    // =========================================================================
    runner.Register("Format", "Single Arg (Integer)", "自研 FormatToString", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = static_cast<int32_t>(i);
                std::string res = compat::detail::FormatToString("Value: {}", val);
                compat::bench::DoNotOptimize(res);
            }
        });

#if COMPAT_HAS_STD_FORMAT
    runner.Register("Format", "Single Arg (Integer)", "原生 std::format", true, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = static_cast<int32_t>(i);
                std::string res = std::format("Value: {}", val);
                compat::bench::DoNotOptimize(res);
            }
        });
#endif

    runner.Register("Format", "Single Arg (Integer)", "傳統 snprintf", false, kDefaultIters,
        [](uint64_t iters) {
            char buf[64];
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = static_cast<int32_t>(i);
                snprintf(buf, sizeof(buf), "Value: %d", val);
                compat::bench::DoNotOptimize(buf);
            }
        });

    // =========================================================================
    // 2. Single Argument Formatting (String)
    // =========================================================================
    runner.Register("Format", "Single Arg (String)", "自研 FormatToString", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                std::string res = compat::detail::FormatToString("Welcome, {}!", "Antigravity");
                compat::bench::DoNotOptimize(res);
            }
        });

#if COMPAT_HAS_STD_FORMAT
    runner.Register("Format", "Single Arg (String)", "原生 std::format", true, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                std::string res = std::format("Welcome, {}!", "Antigravity");
                compat::bench::DoNotOptimize(res);
            }
        });
#endif

    runner.Register("Format", "Single Arg (String)", "傳統 snprintf", false, kDefaultIters,
        [](uint64_t iters) {
            char buf[64];
            for (uint64_t i = 0; i < iters; ++i) {
                snprintf(buf, sizeof(buf), "Welcome, %s!", "Antigravity");
                compat::bench::DoNotOptimize(buf);
            }
        });

    // =========================================================================
    // 3. Multi-Type Mixed Formatting (Integer + Float + String)
    // =========================================================================
    runner.Register("Format", "Multi-arg Mixed (int+float+str)", "自研 FormatToString", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t id = 1000 + static_cast<int32_t>(i % 1000);
                double score = 98.75;
                std::string res = compat::detail::FormatToString("ID: {}, Score: {}, Status: {}", id, score, "VERIFIED");
                compat::bench::DoNotOptimize(res);
            }
        });

#if COMPAT_HAS_STD_FORMAT
    runner.Register("Format", "Multi-arg Mixed (int+float+str)", "原生 std::format", true, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t id = 1000 + static_cast<int32_t>(i % 1000);
                double score = 98.75;
                std::string res = std::format("ID: {}, Score: {}, Status: {}", id, score, "VERIFIED");
                compat::bench::DoNotOptimize(res);
            }
        });
#endif

    runner.Register("Format", "Multi-arg Mixed (int+float+str)", "傳統 snprintf", false, kDefaultIters,
        [](uint64_t iters) {
            char buf[128];
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t id = 1000 + static_cast<int32_t>(i % 1000);
                double score = 98.75;
                snprintf(buf, sizeof(buf), "ID: %d, Score: %.2f, Status: %s", id, score, "VERIFIED");
                compat::bench::DoNotOptimize(buf);
            }
        });
}
