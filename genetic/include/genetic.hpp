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

};

#endif // GENETIC_H
