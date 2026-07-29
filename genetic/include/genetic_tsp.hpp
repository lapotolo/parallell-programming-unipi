#ifndef GENETIC_TSP_H
#define GENETIC_TSP_H

#include "domain.hpp"
#include "executors/executor.hpp"
#include "genetic_operations.hpp"
#include "genetic_pipeline.hpp"
#include "genetic_state.hpp"

#include <cstddef>
#include <utility>

class GeneticTsp
{
public:
  GeneticTsp(GeneticConfig config, FitnessFunction fitness_function)
    : config_{std::move(config)}
    , fitness_function_{std::move(fitness_function)}
    , initialization_engine_{make_random_engine()}
  {
    config_.validate();
    initialize_algorithm_state(
      state_, config_, fitness_function_, initialization_engine_);
  }

  template<typename Executor>
  BestSolution run(Executor& executor)
  {
    run_generations(state_, config_, fitness_function_, executor);
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

private:
  GeneticConfig config_;
  FitnessFunction fitness_function_;
  GeneticState state_;
  RandomEngine initialization_engine_;
};

#endif // GENETIC_TSP_H
