#ifndef GENETIC_TSP_PAR_POOL_H
#define GENETIC_TSP_PAR_POOL_H

#include "genetic.hpp"
#include "genetic_operations.hpp"
#include "partition.hpp"
#include "pool.hpp"

#include <cstddef>
#include <future>
#include <utility>
#include <vector>

class Genetic_TSP_Parallel_Pool : private Genetic_Algorithm
{
public:
  Genetic_TSP_Parallel_Pool(std::size_t worker_count,
                            GeneticConfig config,
                            FitnessFunction fitness_function)
    : Genetic_Algorithm(std::move(config), std::move(fitness_function))
    , worker_count_{worker_count}
    , pool_{worker_count}
    , initialization_engine_{make_random_engine()}
  {
    ranges_ = partition_evenly(config_.population_size, worker_count_);
    pair_ranges_ = partition_evenly(config_.population_size / 2, worker_count_);
    random_engines_.reserve(ranges_.size());
    for(std::size_t worker = 0; worker < ranges_.size(); ++worker)
      random_engines_.push_back(make_random_engine());

    initialize_algorithm_state(
      state_, config_, fitness_function_, initialization_engine_);
  }

  void run()
  {
    for(std::size_t epoch = 0; epoch < config_.epochs; ++epoch)
      next_generation();
  }

  [[nodiscard]] BestSolution get_current_optimum() const
  {
    return state_.global_best;
  }

private:
  std::size_t worker_count_;
  Thread_Pool pool_;
  std::vector<Work_Range> ranges_;
  std::vector<Work_Range> pair_ranges_;
  RandomEngine initialization_engine_;
  std::vector<RandomEngine> random_engines_;

  template<typename Function>
  void run_ranges(const std::vector<Work_Range>& ranges, Function function)
  {
    std::vector<std::future<void>> futures;
    futures.reserve(ranges.size());

    for(std::size_t worker = 0; worker < ranges.size(); ++worker)
    {
      const auto range = ranges[worker];
      futures.push_back(pool_.enqueue([&, worker, range] {
        function(range, random_engines_[worker]);
      }));
    }

    for(auto& future : futures)
      future.get();
  }

  void next_generation()
  {
    run_ranges(pair_ranges_, [&](Work_Range range, RandomEngine& engine) {
      crossover_pair_range(state_, config_, range, engine);
    });

    run_ranges(ranges_, [&](Work_Range range, RandomEngine& engine) {
      mutate_range(state_, config_, range, engine);
    });

    run_ranges(ranges_, [&](Work_Range range, RandomEngine&) {
      evaluate_range(state_, fitness_function_, range);
    });

    update_best_and_apply_elitism(state_);
  }
};

#endif // GENETIC_TSP_PAR_POOL_H
