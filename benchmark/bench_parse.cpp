#include "BenchmarkRunner.hpp"
#include <compat/Parse.hpp>

#include <charconv>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

namespace {
    static constexpr const char* kInt32Raw = "123456789";
    static constexpr const char* kInt64Raw = "922337203685477580";
    static constexpr const char* kDoubleRaw = "3.141592653589793";
    static constexpr const char* kInvalidRaw = "invalid_number_sequence_404";
}

/// <summary>
/// Registers performance benchmarks for type parsing.
/// Compares self-contained compat::parse, native std::from_chars (C++17), and legacy std::stoi / std::stod.
/// </summary>
/// <param name="runner">Benchmark runner instance to register test cases into.</param>
void RegisterParseBenchmarks(compat::bench::BenchmarkRunner& runner) {
    constexpr uint64_t kIntIters = 3000000;
    constexpr uint64_t kFloatIters = 1000000;
    constexpr uint64_t kFailIters = 500000;

    // =========================================================================
    // 1. int32_t Parsing
    // =========================================================================
    const std::string s_int32 = kInt32Raw;

    runner.Register("Parse", "Parse int32_t", "自研 compat::parse", false, kIntIters,
        [](uint64_t iters) {
            compat::string_view sv(kInt32Raw);
            for (uint64_t i = 0; i < iters; ++i) {
                auto res = compat::parse<int32_t>(sv);
                int32_t val = res.value();
                compat::bench::DoNotOptimize(val);
            }
        });

    runner.Register("Parse", "Parse int32_t", "原生 std::from_chars", true, kIntIters,
        [](uint64_t iters) {
            compat::string_view sv(kInt32Raw);
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = 0;
                auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
                compat::bench::DoNotOptimize(val);
                compat::bench::DoNotOptimize(ec);
            }
        });

    runner.Register("Parse", "Parse int32_t", "傳統 std::stoi", false, kIntIters,
        [s_int32](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = std::stoi(s_int32);
                compat::bench::DoNotOptimize(val);
            }
        });

    // =========================================================================
    // 2. int64_t Parsing
    // =========================================================================
    const std::string s_int64 = kInt64Raw;

    runner.Register("Parse", "Parse int64_t", "自研 compat::parse", false, kIntIters,
        [](uint64_t iters) {
            compat::string_view sv(kInt64Raw);
            for (uint64_t i = 0; i < iters; ++i) {
                auto res = compat::parse<int64_t>(sv);
                int64_t val = res.value();
                compat::bench::DoNotOptimize(val);
            }
        });

    runner.Register("Parse", "Parse int64_t", "原生 std::from_chars", true, kIntIters,
        [](uint64_t iters) {
            compat::string_view sv(kInt64Raw);
            for (uint64_t i = 0; i < iters; ++i) {
                int64_t val = 0;
                auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
                compat::bench::DoNotOptimize(val);
                compat::bench::DoNotOptimize(ec);
            }
        });

    runner.Register("Parse", "Parse int64_t", "傳統 std::stoll", false, kIntIters,
        [s_int64](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                int64_t val = std::stoll(s_int64);
                compat::bench::DoNotOptimize(val);
            }
        });

    // =========================================================================
    // 3. double Floating-point Parsing
    // =========================================================================
    const std::string s_double = kDoubleRaw;

    runner.Register("Parse", "Parse double", "自研 compat::parse", false, kFloatIters,
        [](uint64_t iters) {
            compat::string_view sv(kDoubleRaw);
            for (uint64_t i = 0; i < iters; ++i) {
                auto res = compat::parse<double>(sv);
                double val = res.value();
                compat::bench::DoNotOptimize(val);
            }
        });

    runner.Register("Parse", "Parse double", "原生 std::from_chars", true, kFloatIters,
        [](uint64_t iters) {
            compat::string_view sv(kDoubleRaw);
            for (uint64_t i = 0; i < iters; ++i) {
#if defined(__cpp_lib_to_chars) && (__cpp_lib_to_chars >= 201611L)
                double val = 0.0;
                auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
                compat::bench::DoNotOptimize(val);
                compat::bench::DoNotOptimize(ec);
#else
                char* end = nullptr;
                double val = std::strtod(sv.data(), &end);
                compat::bench::DoNotOptimize(val);
#endif
            }
        });

    runner.Register("Parse", "Parse double", "傳統 std::stod", false, kFloatIters,
        [s_double](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                double val = std::stod(s_double);
                compat::bench::DoNotOptimize(val);
            }
        });

    // =========================================================================
    // 4. Failure Input Validation Latency (Fail-Fast vs Exception Overhead)
    // =========================================================================
    const std::string s_invalid = kInvalidRaw;

    runner.Register("Parse", "Failure Input Validation", "自研 compat::parse", false, kFailIters,
        [](uint64_t iters) {
            compat::string_view sv(kInvalidRaw);
            for (uint64_t i = 0; i < iters; ++i) {
                auto res = compat::parse<int32_t>(sv);
                bool ok = res.has_value();
                compat::bench::DoNotOptimize(ok);
            }
        });

    runner.Register("Parse", "Failure Input Validation", "原生 std::from_chars", true, kFailIters,
        [](uint64_t iters) {
            compat::string_view sv(kInvalidRaw);
            for (uint64_t i = 0; i < iters; ++i) {
                int32_t val = 0;
                auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
                bool ok = (ec == std::errc{});
                compat::bench::DoNotOptimize(ok);
            }
        });

    runner.Register("Parse", "Failure Input Validation", "傳統 std::stoi (Exception)", false, kFailIters,
        [s_invalid](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                try {
                    int32_t val = std::stoi(s_invalid);
                    compat::bench::DoNotOptimize(val);
                } catch (const std::exception& e) {
                    compat::bench::DoNotOptimize(e);
                }
            }
        });
}
