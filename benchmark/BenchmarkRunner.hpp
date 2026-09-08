#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <atomic>
#include <sstream>
#include <unordered_map>

namespace compat::bench {

/// <summary>
/// Prevents the compiler from dead-code eliminating computations on a non-const value.
/// </summary>
/// <typeparam name="T">Type of the protected value.</typeparam>
/// <param name="value">Reference to the value to prevent optimization on.</param>
template <typename T>
inline void DoNotOptimize(T& value) noexcept {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#elif defined(__GNUC__)
    asm volatile("" : "+m,r"(value) : : "memory");
#elif defined(_MSC_VER)
    const volatile void* volatile p = &value;
    (void)p;
    std::atomic_signal_fence(std::memory_order_seq_cst);
#else
    const volatile void* volatile p = &value;
    (void)p;
#endif
}

/// <summary>
/// Prevents the compiler from dead-code eliminating computations on a const or temporary value.
/// </summary>
/// <typeparam name="T">Type of the protected value.</typeparam>
/// <param name="value">Const reference to the value to prevent optimization on.</param>
template <typename T>
inline void DoNotOptimize(const T& value) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(value) : "memory");
#elif defined(_MSC_VER)
    const volatile void* volatile p = &value;
    (void)p;
    std::atomic_signal_fence(std::memory_order_seq_cst);
#else
    const volatile void* volatile p = &value;
    (void)p;
#endif
}

/// <summary>
/// Memory clobber barrier preventing memory reordering or dead-store elimination across loop iterations.
/// </summary>
inline void ClobberMemory() noexcept {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : : "memory");
#else
    std::atomic_signal_fence(std::memory_order_seq_cst);
#endif
}

/// <summary>
/// Benchmark runtime configuration parsed from command line options.
/// </summary>
struct BenchmarkConfig {
    uint32_t rounds = 5;
    uint64_t iterations_override = 0;
    std::string filter;
    bool markdown_only = false;
    bool verbose = false;

    /// <summary>
    /// Parses benchmark command-line arguments.
    /// </summary>
    /// <param name="argc">Argument count.</param>
    /// <param name="argv">Argument vector.</param>
    /// <returns>Populated BenchmarkConfig structure.</returns>
    static BenchmarkConfig ParseCommandLine(int argc, char* argv[]) {
        BenchmarkConfig cfg;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--markdown") {
                cfg.markdown_only = true;
            } else if (arg == "--verbose" || arg == "-v") {
                cfg.verbose = true;
            } else if ((arg == "--filter" || arg == "-f") && i + 1 < argc) {
                cfg.filter = argv[++i];
            } else if ((arg == "--rounds" || arg == "-r") && i + 1 < argc) {
                cfg.rounds = static_cast<uint32_t>(std::stoul(argv[++i]));
            } else if ((arg == "--iterations" || arg == "-n") && i + 1 < argc) {
                cfg.iterations_override = std::stoull(argv[++i]);
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Usage: compat_benchmark [options]\n"
                          << "Options:\n"
                          << "  --filter, -f <substr>      Run only benchmarks containing <substr>\n"
                          << "  --rounds, -r <N>           Number of measurement rounds (default: 5)\n"
                          << "  --iterations, -n <N>       Override iteration count for all cases\n"
                          << "  --markdown                 Output strictly Markdown report\n"
                          << "  --verbose, -v              Print detailed per-round progress\n"
                          << "  --help, -h                 Show this help message\n";
                std::exit(0);
            }
        }
        return cfg;
    }
};

/// <summary>
/// Structure containing measurement statistics for a single benchmark test case.
/// </summary>
struct BenchmarkResult {
    std::string suite_name;
    std::string item_name;
    std::string impl_name;
    bool is_baseline = false;
    uint64_t iterations = 0;
    uint32_t rounds = 0;
    double mean_ns_per_op = 0.0;
    double min_ns_per_op = 0.0;
    double max_ns_per_op = 0.0;
    double median_ns_per_op = 0.0;
    double mops_per_sec = 0.0;
};

/// <summary>
/// Signature of benchmark callable executing a specified number of loop iterations.
/// </summary>
using BenchmarkFunc = std::function<void(uint64_t iterations)>;

/// <summary>
/// Represents an executable benchmark test case definition.
/// </summary>
struct BenchmarkCase {
    std::string suite_name;
    std::string item_name;
    std::string impl_name;
    bool is_baseline = false;
    uint64_t default_iterations = 1000000;
    BenchmarkFunc func;
};

/// <summary>
/// High-precision micro-benchmark execution and reporting engine.
/// </summary>
class BenchmarkRunner {
public:
    /// <summary>
    /// Constructs an empty benchmark runner.
    /// </summary>
    BenchmarkRunner() = default;

    /// <summary>
    /// Registers a benchmark case into the runner.
    /// </summary>
    /// <param name="test_case">Benchmark case specification.</param>
    void Register(BenchmarkCase test_case) {
        m_cases.push_back(std::move(test_case));
    }

    /// <summary>
    /// Registers a benchmark case with individual parameters.
    /// </summary>
    /// <param name="suite">Benchmark suite name.</param>
    /// <param name="item">Benchmark item name.</param>
    /// <param name="impl">Implementation description.</param>
    /// <param name="is_baseline">Whether this implementation acts as baseline standard.</param>
    /// <param name="default_iterations">Default iteration count.</param>
    /// <param name="func">Execution lambda accepting iteration count.</param>
    void Register(const std::string& suite,
                  const std::string& item,
                  const std::string& impl,
                  bool is_baseline,
                  uint64_t default_iterations,
                  BenchmarkFunc func) {
        BenchmarkCase c;
        c.suite_name = suite;
        c.item_name = item;
        c.impl_name = impl;
        c.is_baseline = is_baseline;
        c.default_iterations = default_iterations;
        c.func = std::move(func);
        m_cases.push_back(std::move(c));
    }

    /// <summary>
    /// Executes all registered benchmarks according to configuration and prints report.
    /// </summary>
    /// <param name="config">Execution configuration.</param>
    /// <returns>Exit code (0 for success).</returns>
    int Run(const BenchmarkConfig& config) {
        std::vector<BenchmarkResult> results;

        if (!config.markdown_only) {
            std::cout << "================================================================================\n";
            std::cout << "       CPP-Compat Micro-Benchmark Suite (Zero External Dependencies)            \n";
            std::cout << "================================================================================\n";
        }

        for (const auto& c : m_cases) {
            // Apply filter string matching suite, item or impl
            if (!config.filter.empty()) {
                std::string full_name = c.suite_name + "/" + c.item_name + "/" + c.impl_name;
                if (full_name.find(config.filter) == std::string::npos) {
                    continue;
                }
            }

            uint64_t iters = config.iterations_override > 0 ? config.iterations_override : c.default_iterations;
            uint32_t rounds = config.rounds > 0 ? config.rounds : 5;

            if (config.verbose && !config.markdown_only) {
                std::cout << "[RUNNING] " << c.suite_name << " :: " << c.item_name << " (" << c.impl_name << ") ... " << std::flush;
            }

            // 1. Warmup phase (5% of iterations, capped between 1,000 and 100,000)
            uint64_t warmup_iters = (std::max)(static_cast<uint64_t>(1000), (std::min)(iters / 20, static_cast<uint64_t>(100000)));
            c.func(warmup_iters);
            ClobberMemory();

            // 2. Multi-round measurement
            std::vector<double> round_times_ns;
            round_times_ns.reserve(rounds);

            for (uint32_t r = 0; r < rounds; ++r) {
                ClobberMemory();
                auto start = std::chrono::high_resolution_clock::now();
                c.func(iters);
                auto end = std::chrono::high_resolution_clock::now();
                ClobberMemory();

                double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count();
                round_times_ns.push_back(elapsed_ns / static_cast<double>(iters));
            }

            std::sort(round_times_ns.begin(), round_times_ns.end());
            double sum = std::accumulate(round_times_ns.begin(), round_times_ns.end(), 0.0);
            double mean = sum / static_cast<double>(rounds);
            double median = round_times_ns[rounds / 2];
            double min_ns = round_times_ns.front();
            double max_ns = round_times_ns.back();
            double mops = (mean > 0.0) ? (1000.0 / mean) : 0.0;

            BenchmarkResult res;
            res.suite_name = c.suite_name;
            res.item_name = c.item_name;
            res.impl_name = c.impl_name;
            res.is_baseline = c.is_baseline;
            res.iterations = iters;
            res.rounds = rounds;
            res.mean_ns_per_op = mean;
            res.min_ns_per_op = min_ns;
            res.max_ns_per_op = max_ns;
            res.median_ns_per_op = median;
            res.mops_per_sec = mops;
            results.push_back(res);

            if (config.verbose && !config.markdown_only) {
                std::cout << std::fixed << std::setprecision(2) << mean << " ns/op (" << mops << " Mops/s)\n";
            }
        }

        PrintMarkdownReport(results, std::cout);
        return 0;
    }

    /// <summary>
    /// Formats and outputs structured Markdown table report with relative performance ratio against baseline.
    /// </summary>
    /// <param name="results">Vector of benchmark execution results.</param>
    /// <param name="os">Output stream destination.</param>
    void PrintMarkdownReport(const std::vector<BenchmarkResult>& results, std::ostream& os = std::cout) const {
        if (results.empty()) {
            os << "No benchmarks matched the criteria.\n";
            return;
        }

        // Map suite+item to baseline mean_ns
        std::unordered_map<std::string, double> baseline_map;
        for (const auto& r : results) {
            std::string key = r.suite_name + "::" + r.item_name;
            if (r.is_baseline) {
                baseline_map[key] = r.mean_ns_per_op;
            }
        }

        os << "\n";
        os << "| 測試項目 (Benchmark Item) | 實作方案 (Implementation) | 反覆次數 (Iterations) | 平均耗時 (ns/op) | 吞吐量 (Mops/sec) | 相對標準庫差距 |\n";
        os << "| :--- | :--- | :--- | :--- | :--- | :--- |\n";

        for (const auto& r : results) {
            std::string item_display = r.suite_name + ": " + r.item_name;
            std::string key = r.suite_name + "::" + r.item_name;

            std::string ratio_str = "-";
            auto it = baseline_map.find(key);
            if (r.is_baseline) {
                ratio_str = "基準 (1.00x)";
            } else if (it != baseline_map.end() && it->second > 0.0 && r.mean_ns_per_op > 0.0) {
                double ratio = r.mean_ns_per_op / it->second;
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << ratio << "x";
                if (ratio > 1.05) {
                    ss << " (慢 " << std::setprecision(1) << (ratio - 1.0) * 100.0 << "%)";
                } else if (ratio < 0.95) {
                    ss << " (快 " << std::setprecision(1) << (1.0 - ratio) * 100.0 << "%)";
                } else {
                    ss << " (相當)";
                }
                ratio_str = ss.str();
            }

            os << "| " << item_display
               << " | " << r.impl_name
               << " | " << FormatThousands(r.iterations)
               << " | " << std::fixed << std::setprecision(2) << r.mean_ns_per_op << " ns"
               << " | " << std::fixed << std::setprecision(1) << r.mops_per_sec << " Mops/s"
               << " | " << ratio_str
               << " |\n";
        }
        os << "\n";
    }

private:
    /// <summary>
    /// Formats an integer with thousands comma separators.
    /// </summary>
    /// <param name="val">Integer value.</param>
    /// <returns>Formatted comma-separated string.</returns>
    static std::string FormatThousands(uint64_t val) {
        std::string s = std::to_string(val);
        int insert_pos = static_cast<int>(s.length()) - 3;
        while (insert_pos > 0) {
            s.insert(static_cast<size_t>(insert_pos), ",");
            insert_pos -= 3;
        }
        return s;
    }

    std::vector<BenchmarkCase> m_cases;
};

} // namespace compat::bench
