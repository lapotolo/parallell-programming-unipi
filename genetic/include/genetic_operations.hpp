#ifndef GENETIC_OPERATIONS_H
#define GENETIC_OPERATIONS_H

#include "domain.hpp"
#include "genetic_state.hpp"
#include "mutation.hpp"
#include "partition.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using RandomEngine = std::mt19937_64;

inline RandomEngine make_random_engine()
{
  std::random_device device;
  std::seed_seq seed{
    device(), device(), device(), device(),
    device(), device(), device(), device()
  };
  return RandomEngine{seed};
}

inline void evaluate_range(GeneticState& state,
                           const FitnessFunction& fitness_function,
                           Work_Range range)
{
  for(std::size_t index = range.first; index < range.second; ++index)
    state.fitness[index] = fitness_function(state.population[index]);
}

inline void initialize_population(GeneticState& state,
                                  const GeneticConfig& config,
                                  RandomEngine& engine)
{
  state.population.clear();
  state.population.reserve(config.population_size);

  for(std::size_t index = 0; index < config.population_size; ++index)
  {
    Tour tour(config.chromosome_size);
    std::iota(tour.begin(), tour.end(), City{0});
    std::shuffle(tour.begin(), tour.end(), engine);
    state.population.push_back(std::move(tour));
  }

  state.fitness.resize(config.population_size);
}

inline std::size_t initialize_global_best(GeneticState& state)
{
  if(state.population.empty() || state.fitness.empty())
    throw std::logic_error{"cannot initialize the best solution from empty state"};

  const auto best = std::min_element(state.fitness.begin(), state.fitness.end());
  const auto best_index = static_cast<std::size_t>(
    std::distance(state.fitness.begin(), best));
  state.global_best = BestSolution{*best, state.population[best_index]};
  return best_index;
}

inline void initialize_algorithm_state(GeneticState& state,
                                       const GeneticConfig& config,
                                       const FitnessFunction& fitness_function,
                                       RandomEngine& engine)
{
  initialize_population(state, config, engine);
  evaluate_range(state, fitness_function, {0, config.population_size});
  initialize_global_best(state);
}

inline void repair_tour(Tour& tour)
{
  std::vector<std::size_t> occurrences(tour.size(), 0);
  for(const auto city : tour)
  {
    if(city < 0 || static_cast<std::size_t>(city) >= tour.size())
      throw std::logic_error{"crossover produced an out-of-range city"};
    ++occurrences[static_cast<std::size_t>(city)];
  }

  std::vector<City> missing;
  missing.reserve(tour.size());
  for(std::size_t city = 0; city < occurrences.size(); ++city)
  {
    if(occurrences[city] == 0)
      missing.push_back(static_cast<City>(city));
  }

  std::size_t missing_index = 0;
  for(auto& city : tour)
  {
    const auto index = static_cast<std::size_t>(city);
    if(occurrences[index] <= 1) continue;

    --occurrences[index];
    city = missing.at(missing_index++);
  }
}

inline void crossover_pair_range(GeneticState& state,
                                 const GeneticConfig& config,
                                 Work_Range pair_range,
                                 RandomEngine& engine)
{
  std::bernoulli_distribution should_crossover{
    config.crossover_probability};
  std::uniform_int_distribution<std::size_t> left_distribution{
    1, config.chromosome_size / 2 - 1};
  std::uniform_int_distribution<std::size_t> right_distribution{
    config.chromosome_size / 2, config.chromosome_size - 2};

  for(std::size_t pair_index = pair_range.first;
      pair_index < pair_range.second;
      ++pair_index)
  {
    if(!should_crossover(engine)) continue;

    const auto first_index = 2 * pair_index;
    const auto second_index = first_index + 1;
    const auto left = left_distribution(engine);
    const auto right = right_distribution(engine);

    auto& first = state.population[first_index];
    auto& second = state.population[second_index];

    for(std::size_t position = left; position <= right; ++position)
      std::swap(first[position], second[position]);

    repair_tour(first);
    repair_tour(second);
  }
}

inline void mutate_range(GeneticState& state,
                         const GeneticConfig& config,
                         Work_Range range,
                         RandomEngine& engine)
{
  std::bernoulli_distribution should_mutate{config.mutation_probability};
  for(std::size_t index = range.first; index < range.second; ++index)
  {
    if(!should_mutate(engine)) continue;

    const auto positions = draw_distinct_indices(
      config.chromosome_size,
      engine);
    std::swap(state.population[index][positions.first],
              state.population[index][positions.second]);
  }
}

struct FitnessExtrema
{
  std::size_t best_index{};
  std::size_t worst_index{};
};

inline FitnessExtrema find_fitness_extrema(const GeneticState& state,
                                           Work_Range range)
{
  if(range.first >= range.second || range.second > state.fitness.size())
    throw std::invalid_argument{"cannot find extrema in an invalid range"};

  FitnessExtrema extrema{range.first, range.first};
  for(std::size_t index = range.first + 1; index < range.second; ++index)
  {
    if(state.fitness[index] < state.fitness[extrema.best_index])
      extrema.best_index = index;
    if(state.fitness[index] > state.fitness[extrema.worst_index])
      extrema.worst_index = index;
  }
  return extrema;
}

inline std::size_t update_global_best(GeneticState& state,
                                      std::size_t generation_best_index)
{
  if(state.fitness[generation_best_index] < state.global_best.fitness)
  {
    state.global_best = BestSolution{
      state.fitness[generation_best_index],
      state.population[generation_best_index]
    };
  }
  return generation_best_index;
}

inline std::size_t apply_elitism(GeneticState& state,
                                 std::size_t generation_worst_index)
{
  state.fitness[generation_worst_index] = state.global_best.fitness;
  state.population[generation_worst_index] = state.global_best.tour;
  return generation_worst_index;
}

inline FitnessExtrema update_best_and_apply_elitism(GeneticState& state)
{
  const auto extrema = find_fitness_extrema(
    state,
    {0, state.population.size()});
  update_global_best(state, extrema.best_index);
  apply_elitism(state, extrema.worst_index);
  return extrema;
}

#endif // GENETIC_OPERATIONS_H
