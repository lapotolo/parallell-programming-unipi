#ifndef GENETIC_PIPELINE_H
#define GENETIC_PIPELINE_H

#include "domain.hpp"
#include "executors/executor.hpp"
#include "genetic_operations.hpp"
#include "genetic_state.hpp"
#include "random_context.hpp"

#include <cstddef>

// Each execute_ranges call is a phase barrier: crossover completes before
// mutation starts, mutation completes before evaluation, and evaluation
// completes before the serial optimum update and elitist replacement.
template<typename Executor>
void run_generation(GeneticState& state,
                    const GeneticConfig& config,
                    const FitnessFunction& fitness_function,
                    Executor& executor,
                    RandomContext& random_context)
{
  const auto random_plan = random_context.make_generation_plan(config);

  execute_ranges(
    executor,
    config.population_size / 2,
    [&](std::size_t first, std::size_t last, WorkerId) {
      crossover_pair_range(state, {first, last}, random_plan);
    });

  execute_ranges(
    executor,
    config.population_size,
    [&](std::size_t first, std::size_t last, WorkerId) {
      mutate_range(state, {first, last}, random_plan);
    });

  execute_ranges(
    executor,
    config.population_size,
    [&](std::size_t first, std::size_t last, WorkerId) {
      evaluate_range(state, fitness_function, {first, last});
    });

  update_best_and_apply_elitism(state);
}

template<typename Executor>
void run_generations(GeneticState& state,
                     const GeneticConfig& config,
                     const FitnessFunction& fitness_function,
                     Executor& executor,
                     RandomContext& random_context)
{
  for(std::size_t epoch = 0; epoch < config.epochs; ++epoch)
    run_generation(
      state,
      config,
      fitness_function,
      executor,
      random_context);
}

#endif // GENETIC_PIPELINE_H
