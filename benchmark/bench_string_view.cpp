#include "BenchmarkRunner.hpp"
#include <compat/detail/SelfStringView.hpp>

#include <string_view>
#include <string>
#include <cstdint>

namespace {
    static constexpr const char* kRawText = "The quick brown fox jumps over the lazy dog and runs away quickly";
    static constexpr const char* kComp1 = "Modern C++ Zero External Dependency Shim";
    static constexpr const char* kComp2 = "Modern C++ Zero External Dependency Shim";
    static constexpr const char* kMis1 = "Modern C++ Zero External Dependency ShimA";
    static constexpr const char* kMis2 = "Modern C++ Zero External Dependency ShimB";
}

/// <summary>
/// Registers performance benchmarks for string_view operations.
/// Compares self-contained compat::detail::string_view, native std::string_view, and heap-allocating std::string.
/// </summary>
/// <param name="runner">Benchmark runner instance to register test cases into.</param>
void RegisterStringViewBenchmarks(compat::bench::BenchmarkRunner& runner) {
    constexpr uint64_t kDefaultIters = 5000000;
    constexpr uint64_t kHeapIters = 1000000;

    const std::string s_text(kRawText);

    // =========================================================================
    // 1. Substr Slicing Operation
    // =========================================================================
    runner.Register("StringView", "Substr Slice", "自研 detail::string_view", false, kDefaultIters,
        [](uint64_t iters) {
            compat::detail::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto sub = sv.substr(16, 19);
                compat::bench::DoNotOptimize(sub);
            }
        });

    runner.Register("StringView", "Substr Slice", "原生 std::string_view", true, kDefaultIters,
        [](uint64_t iters) {
            std::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto sub = sv.substr(16, 19);
                compat::bench::DoNotOptimize(sub);
            }
        });

    runner.Register("StringView", "Substr Slice", "傳統 std::string (Heap)", false, kHeapIters,
        [s_text](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                std::string sub = s_text.substr(16, 19);
                compat::bench::DoNotOptimize(sub);
            }
        });

    // =========================================================================
    // 2. Find Character Operation
    // =========================================================================
    runner.Register("StringView", "Find Character", "自研 detail::string_view", false, kDefaultIters,
        [](uint64_t iters) {
            compat::detail::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto pos = sv.find('z');
                compat::bench::DoNotOptimize(pos);
            }
        });

    runner.Register("StringView", "Find Character", "原生 std::string_view", true, kDefaultIters,
        [](uint64_t iters) {
            std::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto pos = sv.find('z');
                compat::bench::DoNotOptimize(pos);
            }
        });

    // =========================================================================
    // 3. Find Substring Operation
    // =========================================================================
    runner.Register("StringView", "Find Substring", "自研 detail::string_view", false, kDefaultIters,
        [](uint64_t iters) {
            compat::detail::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto pos = sv.find("lazy dog");
                compat::bench::DoNotOptimize(pos);
            }
        });

    runner.Register("StringView", "Find Substring", "原生 std::string_view", true, kDefaultIters,
        [](uint64_t iters) {
            std::string_view sv(kRawText);
            for (uint64_t i = 0; i < iters; ++i) {
                auto pos = sv.find("lazy dog");
                compat::bench::DoNotOptimize(pos);
            }
        });

    // =========================================================================
    // 4. Operator== Comparison (Identical Strings)
    // =========================================================================
    runner.Register("StringView", "Operator== (Equal)", "自研 detail::string_view", false, kDefaultIters,
        [](uint64_t iters) {
            compat::detail::string_view sv1(kComp1);
            compat::detail::string_view sv2(kComp2);
            for (uint64_t i = 0; i < iters; ++i) {
                bool eq = (sv1 == sv2);
                compat::bench::DoNotOptimize(eq);
            }
        });

    runner.Register("StringView", "Operator== (Equal)", "原生 std::string_view", true, kDefaultIters,
        [](uint64_t iters) {
            std::string_view sv1(kComp1);
            std::string_view sv2(kComp2);
            for (uint64_t i = 0; i < iters; ++i) {
                bool eq = (sv1 == sv2);
                compat::bench::DoNotOptimize(eq);
            }
        });

    // =========================================================================
    // 5. Operator== Comparison (Mismatch at End)
    // =========================================================================
    runner.Register("StringView", "Operator== (Mismatch End)", "自研 detail::string_view", false, kDefaultIters,
        [](uint64_t iters) {
            compat::detail::string_view sv1(kMis1);
            compat::detail::string_view sv2(kMis2);
            for (uint64_t i = 0; i < iters; ++i) {
                bool eq = (sv1 == sv2);
                compat::bench::DoNotOptimize(eq);
            }
        });

    runner.Register("StringView", "Operator== (Mismatch End)", "原生 std::string_view", true, kDefaultIters,
        [](uint64_t iters) {
            std::string_view sv1(kMis1);
            std::string_view sv2(kMis2);
            for (uint64_t i = 0; i < iters; ++i) {
                bool eq = (sv1 == sv2);
                compat::bench::DoNotOptimize(eq);
            }
        });
}
