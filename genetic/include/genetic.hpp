#ifndef GENETIC_H
#define GENETIC_H

#include "domain.hpp"

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
  Population population_;
  FitnessFunction fitness_function_;
  FitnessVector fitness_;
  BestSolution global_best_;

  std::size_t initialize_current_optimum()
  {
    if(population_.empty() || fitness_.empty())
      throw std::logic_error{"cannot initialize the optimum from an empty population"};

    const auto best = std::min_element(fitness_.begin(), fitness_.end());
    const auto best_index = static_cast<std::size_t>(
      std::distance(fitness_.begin(), best));
    global_best_ = BestSolution{*best, population_[best_index]};
    return best_index;
  }
};

#endif // GENETIC_H
