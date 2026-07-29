#ifndef GENETIC_TSP_H
#define GENETIC_TSP_H

#include "domain.hpp"
#include "executors/executor.hpp"
#include "genetic_operations.hpp"
#include "genetic_pipeline.hpp"
#include "genetic_state.hpp"
#include "operation_counts.hpp"
#include "random_context.hpp"

#include <utility>

class GeneticTsp
{
public:
  GeneticTsp(GeneticConfig config,
             FitnessFunction fitness_function,
             RandomSeed seed = make_random_seed())
    : config_{std::move(config)}
    , fitness_function_{std::move(fitness_function)}
    , random_context_{seed}
  {
    config_.validate();
    initialize_algorithm_state(
      state_, config_, fitness_function_, random_context_.engine());
    operation_counts_.fitness_evaluations = config_.population_size;
  }

  template<typename Executor>
  BestSolution run(Executor& executor)
  {
    run_generations(
      state_,
      config_,
      fitness_function_,
      executor,
      random_context_,
      operation_counts_);
    return state_.global_best;
  }

  [[nodiscard]] const GeneticState& state() const noexcept
  {
    return state_;
  }

  [[nodiscard]] const GeneticConfig& config() const noexcept
  {
    return config_;
  }

  [[nodiscard]] const BestSolution& best_solution() const noexcept
  {
    return state_.global_best;
  }

  [[nodiscard]] const OperationCounts& operation_counts() const noexcept
  {
    return operation_counts_;
  }

  [[nodiscard]] RandomSeed seed() const noexcept
  {
    return random_context_.seed();
  }

private:
  GeneticConfig config_;
  FitnessFunction fitness_function_;
  RandomContext random_context_;
  GeneticState state_;
  OperationCounts operation_counts_;
};

#endif // GENETIC_TSP_H
