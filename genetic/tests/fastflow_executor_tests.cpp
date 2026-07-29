#include "../include/executors/fastflow_executor.hpp"
#include "../include/genetic_tsp.hpp"
#include "../include/validation.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>

namespace
{
Fitness tour_fitness(const Tour& tour)
{
  Fitness result = 0;
  for(std::size_t index = 0; index < tour.size(); ++index)
  {
    const auto next = (index + 1) % tour.size();
    const auto difference = tour[index] > tour[next]
                          ? tour[index] - tour[next]
                          : tour[next] - tour[index];
    result += static_cast<Fitness>(difference);
  }
  return result;
}
}

int main()
{
  FastFlowExecutor executor{4};
  GeneticTsp algorithm{GeneticConfig{9, 6, 3}, tour_fitness};
  const auto result = algorithm.run(executor);

  assert(validate_best_solution(result, 6, tour_fitness));
  assert(validate_genetic_state(
    algorithm.state(),
    algorithm.config(),
    tour_fitness));

  bool propagated = false;
  try
  {
    execute_ranges(executor, 8, [](std::size_t first,
                                   std::size_t,
                                   WorkerId) {
      if(first == 0) throw std::runtime_error{"FastFlow task failure"};
    });
  }
  catch(const std::runtime_error&)
  {
    propagated = true;
  }
  assert(propagated);

  std::cout << "FastFlow executor tests passed.\n";
}
