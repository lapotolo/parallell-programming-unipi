#include "../include/domain.hpp"
#include "../include/executors/raw_thread_executor.hpp"
#include "../include/executors/sequential_executor.hpp"
#include "../include/executors/thread_pool_executor.hpp"
#ifndef P1_DISABLE_FASTFLOW
#include "../include/executors/fastflow_executor.hpp"
#endif
#include "../include/genetic_state.hpp"
#include "../include/genetic_tsp.hpp"
#include "../include/operation_counts.hpp"
#include "../include/validation.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

namespace
{
Fitness tour_fitness(const Tour& tour)
{
  Fitness result = 0;
  for(std::size_t index = 0; index < tour.size(); ++index)
  {
    const auto next = (index + 1) % tour.size();
    const auto distance = tour[index] > tour[next]
                        ? tour[index] - tour[next]
                        : tour[next] - tour[index];
    result += static_cast<Fitness>((index + 1) * (distance + 1));
  }
  return result;
}

struct RunSnapshot
{
  GeneticState state;
  OperationCounts counts;
  BestSolution result;
};

template<typename Executor>
RunSnapshot run_backend(const GeneticConfig& config,
                        RandomSeed seed,
                        Executor& executor)
{
  GeneticTsp algorithm{config, tour_fitness, seed};
  const auto result = algorithm.run(executor);

  assert(validate_genetic_state(
    algorithm.state(),
    algorithm.config(),
    tour_fitness));
  assert(result == algorithm.best_solution());

  return RunSnapshot{
    algorithm.state(),
    algorithm.operation_counts(),
    result};
}

void assert_expected_counts(const OperationCounts& counts,
                            const GeneticConfig& config)
{
  assert(counts.generations == config.epochs);
  assert(counts.crossover_pairs_processed ==
         (config.population_size / 2) * config.epochs);
  assert(counts.mutation_candidates_processed ==
         config.population_size * config.epochs);
  assert(counts.fitness_evaluations ==
         config.population_size * (config.epochs + 1));
}

void assert_equivalent(const RunSnapshot& baseline,
                       const RunSnapshot& candidate)
{
  assert(candidate.state == baseline.state);
  assert(candidate.counts == baseline.counts);
  assert(candidate.result == baseline.result);
}
}

int main()
{
  const GeneticConfig config{
    17,
    8,
    4,
    0.75,
    0.40};
  constexpr RandomSeed seed = 0x123456789ABCDEF0ULL;

  SequentialExecutor sequential_executor;
  const auto baseline = run_backend(
    config,
    seed,
    sequential_executor);
  assert_expected_counts(baseline.counts, config);

  RawThreadExecutor raw_thread_executor{4};
  assert_equivalent(
    baseline,
    run_backend(config, seed, raw_thread_executor));

  ThreadPoolExecutor thread_pool_executor{4};
  assert_equivalent(
    baseline,
    run_backend(config, seed, thread_pool_executor));

#ifndef P1_DISABLE_FASTFLOW
  FastFlowExecutor fastflow_executor{4};
  assert_equivalent(
    baseline,
    run_backend(config, seed, fastflow_executor));
#endif

  std::cout << "P1 backend equivalence tests passed.\n";
  return 0;
}
