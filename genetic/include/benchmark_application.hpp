#ifndef BENCHMARK_APPLICATION_H
#define BENCHMARK_APPLICATION_H

#include "domain.hpp"
#include "genetic_tsp.hpp"
#include "random_context.hpp"
#include "tsp_graph.hpp"
#include "validation.hpp"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

struct BenchmarkVariant
{
  std::string_view file_suffix;
  std::string_view output_label;
  std::optional<std::size_t> worker_count;
};

inline std::filesystem::path make_result_path(
  const GeneticConfig& config,
  const BenchmarkVariant& variant)
{
  std::string filename =
    std::to_string(config.epochs) + "-max_epochs-" +
    std::to_string(config.population_size) + "-chromo-" +
    std::to_string(config.chromosome_size) + "-cities";

  if(variant.worker_count)
  {
    filename += "-" + std::to_string(*variant.worker_count) + "-nw";
  }

  filename += "_";
  filename += variant.file_suffix;
  filename += ".data";
  return std::filesystem::path{"results/runs"} / filename;
}

template<typename Executor>
int run_tsp_benchmark(const GeneticConfig& config,
                      RandomSeed seed,
                      Executor& executor,
                      const BenchmarkVariant& variant)
{
  const TspGraph graph{
    config.chromosome_size,
    derive_seed(seed, RandomSeed{0})};
  const FitnessFunction fitness_function = [&graph](const Tour& tour) {
    return graph.tour_cost(tour);
  };

  GeneticTsp algorithm{
    config,
    fitness_function,
    derive_seed(seed, RandomSeed{1})};

  std::cerr << "seed=" << seed << '\n';

  const auto start = std::chrono::steady_clock::now();
  const auto best_solution = algorithm.run(executor);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  const auto microseconds = std::chrono::duration_cast<
    std::chrono::microseconds>(elapsed).count();

  if(!validate_best_solution(
       best_solution,
       config.chromosome_size,
       fitness_function))
  {
    return -1;
  }

  const auto result_path = make_result_path(config, variant);
  std::filesystem::create_directories(result_path.parent_path());
  std::ofstream output{result_path, std::ios::app};
  if(!output)
    throw std::runtime_error{"cannot open benchmark result file"};
  output << microseconds << '\n';

  std::cout << variant.output_label;
  if(variant.worker_count)
    std::cout << '(' << *variant.worker_count << ')';
  std::cout << '=' << microseconds << '\n';
  return 0;
}

#endif // BENCHMARK_APPLICATION_H
