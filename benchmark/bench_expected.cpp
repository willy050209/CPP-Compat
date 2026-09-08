#define COMPAT_ENABLE_UNION_EXPECTED 1
#define COMPAT_ENABLE_VARIANT_EXPECTED 1
#define COMPAT_BENCHMARK_ISOLATE_EXPECTED 1

#include "BenchmarkRunner.hpp"
#include <compat/Config.hpp>
#include <compat/detail/SelfUnionExpected.hpp>
#include <compat/detail/SelfExpected.hpp>

#if COMPAT_HAS_STD_EXPECTED
#  include <expected>
#endif

#include <cstdint>

/// <summary>
/// Registers performance benchmarks for expected and unexpected types.
/// Compares self-contained union fallback, variant fallback, and native std::expected (C++23).
/// </summary>
/// <param name="runner">Benchmark runner instance to register test cases into.</param>
void RegisterExpectedBenchmarks(compat::bench::BenchmarkRunner& runner) {
    constexpr uint64_t kDefaultIters = 5000000;

    // =========================================================================
    // 1. Create & Access Success Value
    // =========================================================================
    runner.Register("Expected", "Create & Access Success", "自研 Fallback (Union)", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                compat::detail::union_impl::expected<int32_t, int32_t> exp(static_cast<int32_t>(i));
                auto val = exp.value();
                compat::bench::DoNotOptimize(val);
            }
        });

    runner.Register("Expected", "Create & Access Success", "自研 Fallback (Variant)", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                compat::detail::variant_impl::expected<int32_t, int32_t> exp(static_cast<int32_t>(i));
                auto val = exp.value();
                compat::bench::DoNotOptimize(val);
            }
        });

#if COMPAT_HAS_STD_EXPECTED
    runner.Register("Expected", "Create & Access Success", "原生 std::expected", true, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                std::expected<int32_t, int32_t> exp(static_cast<int32_t>(i));
                auto val = exp.value();
                compat::bench::DoNotOptimize(val);
            }
        });
#endif

    // =========================================================================
    // 2. Create & Access Error State
    // =========================================================================
    runner.Register("Expected", "Create & Access Error", "自研 Fallback (Union)", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                compat::detail::union_impl::expected<int32_t, int32_t> exp(
                    compat::detail::union_impl::unexpected<int32_t>(static_cast<int32_t>(i)));
                auto err = exp.error();
                compat::bench::DoNotOptimize(err);
            }
        });

    runner.Register("Expected", "Create & Access Error", "自研 Fallback (Variant)", false, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                compat::detail::variant_impl::expected<int32_t, int32_t> exp(
                    compat::detail::variant_impl::unexpected<int32_t>(static_cast<int32_t>(i)));
                auto err = exp.error();
                compat::bench::DoNotOptimize(err);
            }
        });

#if COMPAT_HAS_STD_EXPECTED
    runner.Register("Expected", "Create & Access Error", "原生 std::expected", true, kDefaultIters,
        [](uint64_t iters) {
            for (uint64_t i = 0; i < iters; ++i) {
                std::expected<int32_t, int32_t> exp(
                    std::unexpected<int32_t>(static_cast<int32_t>(i)));
                auto err = exp.error();
                compat::bench::DoNotOptimize(err);
            }
        });
#endif

    // =========================================================================
    // 3. Boolean Check & Conditional Branching
    // =========================================================================
    runner.Register("Expected", "Boolean Check & Branch", "自研 Fallback (Union)", false, kDefaultIters,
        [](uint64_t iters) {
            int32_t sum = 0;
            for (uint64_t i = 0; i < iters; ++i) {
                // Alternate between success and error states
                bool is_val = (i & 1) != 0;
                compat::detail::union_impl::expected<int32_t, int32_t> exp = is_val ?
                    compat::detail::union_impl::expected<int32_t, int32_t>(static_cast<int32_t>(i)) :
                    compat::detail::union_impl::expected<int32_t, int32_t>(
                        compat::detail::union_impl::unexpected<int32_t>(static_cast<int32_t>(i)));
                if (exp.has_value()) {
                    sum += exp.value();
                } else {
                    sum -= exp.error();
                }
                compat::bench::DoNotOptimize(sum);
            }
        });

    runner.Register("Expected", "Boolean Check & Branch", "自研 Fallback (Variant)", false, kDefaultIters,
        [](uint64_t iters) {
            int32_t sum = 0;
            for (uint64_t i = 0; i < iters; ++i) {
                bool is_val = (i & 1) != 0;
                compat::detail::variant_impl::expected<int32_t, int32_t> exp = is_val ?
                    compat::detail::variant_impl::expected<int32_t, int32_t>(static_cast<int32_t>(i)) :
                    compat::detail::variant_impl::expected<int32_t, int32_t>(
                        compat::detail::variant_impl::unexpected<int32_t>(static_cast<int32_t>(i)));
                if (exp.has_value()) {
                    sum += exp.value();
                } else {
                    sum -= exp.error();
                }
                compat::bench::DoNotOptimize(sum);
            }
        });

#if COMPAT_HAS_STD_EXPECTED
    runner.Register("Expected", "Boolean Check & Branch", "原生 std::expected", true, kDefaultIters,
        [](uint64_t iters) {
            int32_t sum = 0;
            for (uint64_t i = 0; i < iters; ++i) {
                bool is_val = (i & 1) != 0;
                std::expected<int32_t, int32_t> exp = is_val ?
                    std::expected<int32_t, int32_t>(static_cast<int32_t>(i)) :
                    std::expected<int32_t, int32_t>(std::unexpected<int32_t>(static_cast<int32_t>(i)));
                if (exp.has_value()) {
                    sum += exp.value();
                } else {
                    sum -= exp.error();
                }
                compat::bench::DoNotOptimize(sum);
            }
        });
#endif
}
