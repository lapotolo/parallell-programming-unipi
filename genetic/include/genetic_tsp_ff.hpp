#ifndef GENETIC_TSP_FF_H
#define GENETIC_TSP_FF_H

#include "genetic.hpp"
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
    init_population();
    state_.fitness.resize(config_.population_size);
    for(size_t i = 0; i < config_.population_size; ++i)
      state_.fitness[i] = fitness_function_(state_.population[i]);
    initialize_current_optimum();
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
  size_t num_workers;
  size_t chunks_size; // number of chromosome that each worker have to deal with

  void init_population()
  {
    size_t i;
    state_.population.reserve(config_.population_size);
    for(i = 0; i < config_.population_size; ++i)
    {
      Tour chromosome(config_.chromosome_size);
      std::iota(chromosome.begin(), chromosome.end(), 0);
      std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});
      state_.population.emplace_back(chromosome);
    }
  }
};

#endif // GENETIC_TSP_FF_H