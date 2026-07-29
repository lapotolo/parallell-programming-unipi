#ifndef GENETIC_STATE_H
#define GENETIC_STATE_H

#include "domain.hpp"

#include <stdexcept>

struct GeneticState
{
  Population population;
  FitnessVector fitness;
  BestSolution global_best;
};

inline bool operator==(const GeneticState& first, const GeneticState& second)
{
  return first.population == second.population &&
         first.fitness == second.fitness &&
         first.global_best == second.global_best;
}

inline bool operator!=(const GeneticState& first, const GeneticState& second)
{
  return !(first == second);
}

inline void validate_state_shape(const GeneticState& state,
                                 const GeneticConfig& config)
{
  if(state.population.size() != config.population_size)
    throw std::logic_error{"population size does not match the configuration"};
  if(state.fitness.size() != state.population.size())
    throw std::logic_error{"population and fitness sizes differ"};
}

#endif // GENETIC_STATE_H
