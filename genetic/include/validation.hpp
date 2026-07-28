#ifndef VALIDATION_H
#define VALIDATION_H

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

inline bool is_valid_tour(const std::vector<int>& tour, std::size_t city_count)
{
  if(tour.size() != city_count) return false;

  std::vector<int> expected(city_count);
  std::iota(expected.begin(), expected.end(), 0);

  auto sorted = tour;
  std::sort(sorted.begin(), sorted.end());
  return sorted == expected;
}

template<typename Fitness, typename FitnessFunction>
bool validate_best_solution(const std::pair<Fitness, std::vector<int>>& solution,
                            std::size_t city_count,
                            FitnessFunction&& fitness_function)
{
  if(!is_valid_tour(solution.second, city_count))
  {
    std::cerr << "Invalid best tour: it is not a permutation of all cities.\n";
    return false;
  }

  const auto recomputed = fitness_function(solution.second);
  if(recomputed != solution.first)
  {
    std::cerr << "Invalid best solution: stored fitness " << solution.first
              << " differs from recomputed fitness " << recomputed << ".\n";
    return false;
  }

  return true;
}

#endif // VALIDATION_H
