#include "../include/genetic.hpp"
#include "../include/genetic_tsp_par.hpp"
#include "../include/genetic_tsp_pool.hpp"
#include "../include/genetic_tsp_seq.hpp"
#include "../include/mutation.hpp"
#include "../include/partition.hpp"
#include "../include/validation.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

namespace
{
using Population = std::vector<std::vector<int>>;
using Tour = std::vector<int>;

Fitness tour_fitness(const Tour& tour)
{
  Fitness result = 0;
  for(std::size_t i = 0; i < tour.size(); ++i)
  {
    const auto next = (i + 1) % tour.size();
    const auto difference = tour[i] > tour[next]
                          ? tour[i] - tour[next]
                          : tour[next] - tour[i];
    result += static_cast<Fitness>(difference);
  }
  return result;
}

void check_partition(std::size_t elements, std::size_t workers)
{
  const auto ranges = partition_evenly(elements, workers);
  std::vector<unsigned int> coverage(elements, 0);
  std::size_t previous_end = 0;

  for(const auto& range : ranges)
  {
    assert(range.first == previous_end);
    assert(range.first < range.second);
    assert(range.second <= elements);
    for(std::size_t i = range.first; i < range.second; ++i) ++coverage[i];
    previous_end = range.second;
  }

  assert(previous_end == elements);
  for(const auto count : coverage) assert(count == 1);
}

class Initial_Optimum_Harness
  : public Genetic_Algorithm<Population, Tour, Fitness>
{
public:
  Initial_Optimum_Harness()
    : Genetic_Algorithm(0, 3, 4, tour_fitness)
  {
    population = {{0, 1, 2, 3}, {0, 2, 1, 3}, {0, 3, 1, 2}};
    chromosomes_fitness = {90, 25, 60};
  }

  std::size_t initialize()
  {
    return initialize_current_optimum();
  }

  const std::pair<Fitness, Tour>& optimum() const
  {
    return current_optimum;
  }
};

template<typename AlgorithmFactory>
void check_algorithm_evaluates_every_generation(AlgorithmFactory&& make_algorithm,
                                                std::size_t population_size,
                                                std::size_t epochs,
                                                std::atomic<std::size_t>& calls)
{
  auto algorithm = make_algorithm();
  algorithm.run();
  assert(calls.load() == population_size * (epochs + 1));
  assert(validate_best_solution(algorithm.get_current_optimum(), 6, tour_fitness));
}
}

int main()
{
  check_partition(0, 1);
  check_partition(1, 1);
  check_partition(3, 1);
  check_partition(3, 2);
  check_partition(3, 3);
  check_partition(3, 8);
  check_partition(10, 3);
  check_partition(1000, 8);

  {
    const auto ranges = partition_crossover_aligned(10, 3);
    assert(ranges.size() == 3);
    assert(ranges[0].chromosomes == Work_Range(0, 4));
    assert(ranges[1].chromosomes == Work_Range(4, 8));
    assert(ranges[2].chromosomes == Work_Range(8, 10));
    assert(ranges[0].pairs == Work_Range(0, 2));
    assert(ranges[1].pairs == Work_Range(2, 4));
    assert(ranges[2].pairs == Work_Range(4, 5));
  }
  {
    const auto ranges = partition_crossover_aligned(9, 3);
    assert(ranges.back().chromosomes.second == 9);
    std::size_t previous_end = 0;
    for(const auto& range : ranges)
    {
      assert(range.chromosomes.first == previous_end);
      previous_end = range.chromosomes.second;
    }
    assert(previous_end == 9);
  }

  {
    Initial_Optimum_Harness harness;
    assert(harness.initialize() == 1);
    assert(harness.optimum().first == 25);
    assert(harness.optimum().second == Tour({0, 2, 1, 3}));
  }

  {
    std::mt19937_64 engine{12345};
    for(std::size_t i = 0; i < 1000; ++i)
    {
      const auto positions = draw_distinct_indices(6, engine);
      assert(positions.first < 6);
      assert(positions.second < 6);
      assert(positions.first != positions.second);
    }
  }

  assert(is_valid_tour({0, 1, 2, 3}, 4));
  assert(!is_valid_tour({0, 1, 1, 3}, 4));
  assert(validate_best_solution(std::make_pair(tour_fitness(Tour{0, 1, 2, 3}),
                                                Tour{0, 1, 2, 3}),
                                4,
                                tour_fitness));

  constexpr std::size_t population_size = 9;
  constexpr std::size_t epochs = 3;

  {
    std::atomic<std::size_t> calls{0};
    auto fitness = [&calls](const Tour& tour) {
      ++calls;
      return tour_fitness(tour);
    };
    check_algorithm_evaluates_every_generation(
      [&] { return Genetic_TSP_Sequential(epochs, population_size, 6, fitness); },
      population_size,
      epochs,
      calls);
  }

  {
    std::atomic<std::size_t> calls{0};
    auto fitness = [&calls](const Tour& tour) {
      ++calls;
      return tour_fitness(tour);
    };
    check_algorithm_evaluates_every_generation(
      [&] { return Genetic_TSP_Parallel(4, epochs, population_size, 6, fitness); },
      population_size,
      epochs,
      calls);
  }

  {
    std::atomic<std::size_t> calls{0};
    auto fitness = [&calls](const Tour& tour) {
      ++calls;
      return tour_fitness(tour);
    };
    check_algorithm_evaluates_every_generation(
      [&] { return Genetic_TSP_Parallel_Pool(4, epochs, population_size, 6, fitness); },
      population_size,
      epochs,
      calls);
  }

  std::cout << "P0 correctness tests passed.\n";
  return 0;
}
