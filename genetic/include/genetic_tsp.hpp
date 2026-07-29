#ifndef GENETIC_TSP_H
#define GENETIC_TSP_H

#include "domain.hpp"
#include "executors/executor.hpp"
#include "genetic_operations.hpp"
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
    for(std::size_t epoch = 0; epoch < config_.epochs; ++epoch)
    {
      execute_ranges(
        executor,
        config_.population_size / 2,
        [&](std::size_t first, std::size_t last, WorkerId) {
          auto engine = make_random_engine();
          crossover_pair_range(state_, config_, {first, last}, engine);
        });

      execute_ranges(
        executor,
        config_.population_size,
        [&](std::size_t first, std::size_t last, WorkerId) {
          auto engine = make_random_engine();
          mutate_range(state_, config_, {first, last}, engine);
        });

      execute_ranges(
        executor,
        config_.population_size,
        [&](std::size_t first, std::size_t last, WorkerId) {
          evaluate_range(state_, fitness_function_, {first, last});
        });

      update_best_and_apply_elitism(state_);
    }

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
