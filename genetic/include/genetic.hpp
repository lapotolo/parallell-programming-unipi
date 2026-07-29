#ifndef GENETIC_H
#define GENETIC_H

#include "domain.hpp"
#include "genetic_state.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>

class Genetic_Algorithm
{
public:
  Genetic_Algorithm(GeneticConfig config, FitnessFunction fitness_function)
    : config_{std::move(config)}
    , fitness_function_{std::move(fitness_function)}
  {
    config_.validate();
  }

protected:
  GeneticConfig config_;
  FitnessFunction fitness_function_;
  GeneticState state_;

  std::size_t initialize_current_optimum()
  {
    if(state_.population.empty() || state_.fitness.empty())
      throw std::logic_error{"cannot initialize the optimum from an empty population"};

    const auto best = std::min_element(state_.fitness.begin(), state_.fitness.end());
    const auto best_index = static_cast<std::size_t>(
      std::distance(state_.fitness.begin(), best));
    state_.global_best = BestSolution{*best, state_.population[best_index]};
    return best_index;
  }
};

#endif // GENETIC_H
