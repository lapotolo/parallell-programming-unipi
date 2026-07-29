#include "../include/domain.hpp"
#include "../include/executors/executor.hpp"
#include "../include/executors/raw_thread_executor.hpp"
#include "../include/executors/sequential_executor.hpp"
#include "../include/executors/thread_pool_executor.hpp"
#include "../include/genetic_operations.hpp"
#include "../include/genetic_tsp.hpp"
#include "../include/mutation.hpp"
#include "../include/partition.hpp"
#include "../include/random_context.hpp"
#include "../include/validation.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

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
    for(std::size_t index = range.first; index < range.second; ++index)
      ++coverage[index];
    previous_end = range.second;
  }

  assert(previous_end == elements);
  for(const auto count : coverage)
    assert(count == 1);
}

class RecordingExecutor
{
public:
  template<typename Function>
  void for_each_range(std::size_t count, Function&& function)
  {
    if(count == 0) return;
    ++calls;
    function(0, count, WorkerId{0});
  }

  std::size_t calls = 0;
};

template<typename Executor>
void check_algorithm_evaluates_every_generation(
  Executor& executor,
  std::size_t population_size,
  std::size_t epochs)
{
  std::atomic<std::size_t> calls{0};
  const FitnessFunction fitness = [&calls](const Tour& tour) {
    ++calls;
    return tour_fitness(tour);
  };

  GeneticTsp algorithm{
    GeneticConfig{population_size, 6, epochs},
    fitness,
    RandomSeed{12345}};
  const auto result = algorithm.run(executor);

  assert(calls.load() == population_size * (epochs + 1));
  assert(validate_best_solution(result, 6, fitness));
  assert(validate_genetic_state(
    algorithm.state(),
    algorithm.config(),
    fitness));
}
}

int main()
{
  {
    RecordingExecutor executor;
    std::size_t visited = 0;
    execute_ranges(
      executor,
      7,
      [&](std::size_t first,
          std::size_t last,
          WorkerId worker_id) {
        assert(first == 0);
        assert(last == 7);
        assert(worker_id == 0);
        visited += last - first;
      });
    assert(executor.calls == 1);
    assert(visited == 7);
  }

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
    assert(ranges[0].chromosomes == WorkRange(0, 4));
    assert(ranges[1].chromosomes == WorkRange(4, 8));
    assert(ranges[2].chromosomes == WorkRange(8, 10));
    assert(ranges[0].pairs == WorkRange(0, 2));
    assert(ranges[1].pairs == WorkRange(2, 4));
    assert(ranges[2].pairs == WorkRange(4, 5));
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
    GeneticState state{
      Population{
        Tour{0, 1, 2, 3},
        Tour{0, 2, 1, 3},
        Tour{0, 3, 1, 2}},
      FitnessVector{90, 25, 60},
      BestSolution{}};

    assert(initialize_global_best(state) == 1);
    assert(state.global_best.fitness == 25);
    assert(state.global_best.tour == Tour({0, 2, 1, 3}));
  }

  {
    RandomContext first{12345};
    RandomContext second{12345};
    const GeneticConfig config{9, 6, 3};
    assert(first.make_generation_plan(config) ==
           second.make_generation_plan(config));
  }

  {
    std::mt19937_64 engine{12345};
    for(std::size_t iteration = 0; iteration < 1000; ++iteration)
    {
      const auto positions = draw_distinct_indices(6, engine);
      assert(positions.first < 6);
      assert(positions.second < 6);
      assert(positions.first != positions.second);
    }
  }

  assert(is_valid_tour({0, 1, 2, 3}, 4));
  assert(!is_valid_tour({0, 1, 1, 3}, 4));
  assert(validate_best_solution(
    BestSolution{
      tour_fitness(Tour{0, 1, 2, 3}),
      Tour{0, 1, 2, 3}},
    4,
    tour_fitness));

  {
    GeneticState state{
      Population{Tour{0, 1, 2, 3}},
      FitnessVector{tour_fitness(Tour{0, 1, 2, 3})},
      BestSolution{
        tour_fitness(Tour{0, 1, 2, 3}),
        Tour{0, 1, 2, 3}}
    };
    assert(validate_genetic_state(
      state,
      GeneticConfig{1, 4, 0},
      tour_fitness));
  }

  constexpr std::size_t population_size = 9;
  constexpr std::size_t epochs = 3;

  {
    SequentialExecutor executor;
    check_algorithm_evaluates_every_generation(
      executor,
      population_size,
      epochs);
  }

  {
    RawThreadExecutor executor{4};
    bool propagated = false;
    try
    {
      execute_ranges(
        executor,
        8,
        [](std::size_t first, std::size_t, WorkerId) {
          if(first == 0)
            throw std::runtime_error{"worker failure"};
        });
    }
    catch(const std::runtime_error&)
    {
      propagated = true;
    }
    assert(propagated);

    check_algorithm_evaluates_every_generation(
      executor,
      population_size,
      epochs);
  }

  {
    ThreadPoolExecutor executor{4};
    bool propagated = false;
    try
    {
      execute_ranges(
        executor,
        8,
        [](std::size_t first, std::size_t, WorkerId) {
          if(first == 0)
            throw std::runtime_error{"pool task failure"};
        });
    }
    catch(const std::runtime_error&)
    {
      propagated = true;
    }
    assert(propagated);

    check_algorithm_evaluates_every_generation(
      executor,
      population_size,
      epochs);
  }

  std::cout << "P0 correctness tests passed.\n";
  return 0;
}
