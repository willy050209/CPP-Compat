#include "BenchmarkRunner.hpp"

#include <iostream>

/// <summary>
/// Forward declaration for expected benchmark suite registration.
/// </summary>
/// <param name="runner">Benchmark runner instance.</param>
void RegisterExpectedBenchmarks(compat::bench::BenchmarkRunner& runner);

/// <summary>
/// Forward declaration for format benchmark suite registration.
/// </summary>
/// <param name="runner">Benchmark runner instance.</param>
void RegisterFormatBenchmarks(compat::bench::BenchmarkRunner& runner);

/// <summary>
/// Forward declaration for parse benchmark suite registration.
/// </summary>
/// <param name="runner">Benchmark runner instance.</param>
void RegisterParseBenchmarks(compat::bench::BenchmarkRunner& runner);

/// <summary>
/// Forward declaration for string_view benchmark suite registration.
/// </summary>
/// <param name="runner">Benchmark runner instance.</param>
void RegisterStringViewBenchmarks(compat::bench::BenchmarkRunner& runner);

/// <summary>
/// Entry point for CPP-Compat micro-benchmark executable.
/// Registers all suites, parses CLI arguments, runs iterations, and formats Markdown report.
/// </summary>
/// <param name="argc">Argument count.</param>
/// <param name="argv">Argument vector.</param>
/// <returns>Execution return code (0 on success).</returns>
int main(int argc, char* argv[]) {
    // Parse configuration flags
    compat::bench::BenchmarkConfig config = compat::bench::BenchmarkConfig::ParseCommandLine(argc, argv);
    compat::bench::BenchmarkRunner runner;

    // Register all benchmark modules
    RegisterExpectedBenchmarks(runner);
    RegisterFormatBenchmarks(runner);
    RegisterParseBenchmarks(runner);
    RegisterStringViewBenchmarks(runner);

    // Execute registered benchmarks and emit Markdown table
    return runner.Run(config);
}
