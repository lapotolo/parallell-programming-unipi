#ifndef VALIDATION_H
#define VALIDATION_H

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <utility>
#include "domain.hpp"


inline bool is_valid_tour(const Tour& tour, std::size_t city_count)
{
  if(tour.size() != city_count) return false;

  Tour expected(city_count);
  std::iota(expected.begin(), expected.end(), 0);

  auto sorted = tour;
  std::sort(sorted.begin(), sorted.end());
  return sorted == expected;
}

inline bool validate_best_solution(const BestSolution& solution,
                                   std::size_t city_count,
                                   const FitnessFunction& fitness_function)
{
  if(!is_valid_tour(solution.tour, city_count))
  {
    std::cerr << "Invalid best tour: it is not a permutation of all cities.\n";
    return false;
  }

  const auto recomputed = fitness_function(solution.tour);
  if(recomputed != solution.fitness)
  {
    std::cerr << "Invalid best solution: stored fitness " << solution.fitness
              << " differs from recomputed fitness " << recomputed << ".\n";
    return false;
  }

  return true;
}

#endif // VALIDATION_H
