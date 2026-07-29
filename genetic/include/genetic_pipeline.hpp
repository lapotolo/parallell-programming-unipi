#ifndef GENETIC_PIPELINE_H
#define GENETIC_PIPELINE_H

#include "domain.hpp"
#include "executors/executor.hpp"
#include "genetic_operations.hpp"
#include "genetic_state.hpp"
#include "operation_counts.hpp"
#include "random_context.hpp"

#include <cstddef>

template<typename Executor>
void run_generation(GeneticState& state,
                    const GeneticConfig& config,
                    const FitnessFunction& fitness_function,
                    Executor& executor,
                    RandomContext& random_context,
                    OperationCounts& operation_counts)
{
  const auto random_plan = random_context.make_generation_plan(config);

  execute_ranges(
    executor,
    config.population_size / 2,
    [&](std::size_t first, std::size_t last, WorkerId) {
      crossover_pair_range(state, {first, last}, random_plan);
    });
  operation_counts.crossover_pairs_processed +=
    config.population_size / 2;

  execute_ranges(
    executor,
    config.population_size,
    [&](std::size_t first, std::size_t last, WorkerId) {
      mutate_range(state, {first, last}, random_plan);
    });
  operation_counts.mutation_candidates_processed +=
    config.population_size;

  execute_ranges(
    executor,
    config.population_size,
    [&](std::size_t first, std::size_t last, WorkerId) {
      evaluate_range(state, fitness_function, {first, last});
    });
  operation_counts.fitness_evaluations += config.population_size;

  update_best_and_apply_elitism(state);
  ++operation_counts.generations;
}

template<typename Executor>
void run_generations(GeneticState& state,
                     const GeneticConfig& config,
                     const FitnessFunction& fitness_function,
                     Executor& executor,
                     RandomContext& random_context,
                     OperationCounts& operation_counts)
{
  for(std::size_t epoch = 0; epoch < config.epochs; ++epoch)
  {
    run_generation(
      state,
      config,
      fitness_function,
      executor,
      random_context,
      operation_counts);
  }
}

#endif // GENETIC_PIPELINE_H
