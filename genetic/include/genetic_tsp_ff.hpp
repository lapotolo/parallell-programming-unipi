#ifndef GENETIC_TSP_FF_H
#define GENETIC_TSP_FF_H

#include "genetic.hpp"
#include "genetic_operations.hpp"
#include "ff_farm_tsp.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <numeric>
#include <random>
#include <utility>
#include <vector>




class Genetic_TSP_FF : Genetic_Algorithm
{
public:
  // constructor. First generation is composed of random (feasible) chromosomes
  Genetic_TSP_FF(std::size_t worker_count,
                 GeneticConfig config,
                 FitnessFunction fitness_function)
                : Genetic_Algorithm(std::move(config), std::move(fitness_function))
                , num_workers(worker_count)
  {
    initialize_algorithm_state(
      state_, config_, fitness_function_, initialization_engine_);
  }


  void run()
  {
    if(config_.epochs == 0) return;

    auto shared_state = std::make_shared<GeneticState>(state_);
    auto shared_fitness_function =
      std::make_shared<FitnessFunction>(fitness_function_);

    TSP_Master master(num_workers,
                      config_.epochs,
                      config_.population_size,
                      shared_state,
                      shared_fitness_function,
                      config_);

    std::vector<std::unique_ptr<ff::ff_node>> tsp_workers;
    tsp_workers.reserve(num_workers);
    for(std::size_t worker = 0; worker < num_workers; ++worker)
      tsp_workers.push_back(ff::make_unique<TSP_Worker>());

    ff::ff_Farm<TSP_Task> farm(std::move(tsp_workers), master);
    farm.remove_collector();
    farm.wrap_around();

    if(farm.run_and_wait_end() < 0)
    {
      ff::error("running farm");
      return;
    }

    state_ = std::move(*shared_state);
  }

  BestSolution get_current_optimum() const { return state_.global_best; }

private:
  std::size_t num_workers;
  RandomEngine initialization_engine_{make_random_engine()};

};

#endif // GENETIC_TSP_FF_H