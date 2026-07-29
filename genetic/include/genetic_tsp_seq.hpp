#ifndef GENETIC_TSP_SEQ_H
#define GENETIC_TSP_SEQ_H

#include "genetic.hpp"
#include "genetic_operations.hpp"

#include <cstddef>
#include <utility>

class Genetic_TSP_Sequential : private Genetic_Algorithm
{
public:
  Genetic_TSP_Sequential(GeneticConfig config,
                         FitnessFunction fitness_function)
    : Genetic_Algorithm(std::move(config), std::move(fitness_function))
    , random_engine_{make_random_engine()}
  {
    initialize_algorithm_state(
      state_, config_, fitness_function_, random_engine_);
  }

  void run()
  {
    for(std::size_t epoch = 0; epoch < config_.epochs; ++epoch)
    {
      crossover_pair_range(
        state_, config_, {0, config_.population_size / 2}, random_engine_);
      mutate_range(
        state_, config_, {0, config_.population_size}, random_engine_);
      evaluate_range(
        state_, fitness_function_, {0, config_.population_size});
      update_best_and_apply_elitism(state_);
    }
  }

  [[nodiscard]] BestSolution get_current_optimum() const
  {
    return state_.global_best;
  }

private:
  RandomEngine random_engine_;
};

#endif // GENETIC_TSP_SEQ_H
