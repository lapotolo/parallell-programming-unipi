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
    fitness_.resize(config_.population_size);
    for(size_t i = 0; i < config_.population_size; ++i)
      fitness_[i] = fitness_function_(population_[i]);
    initialize_current_optimum();
  }


  void run() // FF is deployed in here
  {
  if(config_.epochs == 0) return;

  size_t i;
  auto shared_population = std::make_shared<Population>(population_);
  auto shared_fitness = std::make_shared<FitnessVector>(fitness_);
  auto shared_fitness_function_ = std::make_shared<FitnessFunction>(fitness_function_);
  auto shared_optimum = std::make_shared<BestSolution>(global_best_);

  TSP_Master master (num_workers
                   , config_.epochs
                   , config_.population_size
                   , shared_population
                   , shared_fitness
                   , shared_fitness_function_
                   , shared_optimum
                   , config_
                   );

  // create the vector keeping pointers for farm's workers
  std::vector<std::unique_ptr<ff::ff_node>> tsp_workers;
  for(i = 0; i < num_workers; ++i)
    tsp_workers.push_back(ff::make_unique<TSP_Worker>());

  // create the farm and set its topology (Master-Worker)
  ff::ff_Farm<TSP_Task> farm_gene_tsp(std::move(tsp_workers), master);
  farm_gene_tsp.remove_collector();
  farm_gene_tsp.wrap_around();

  // run the farm
  //ff::ffTime(ff::START_TIME);
  if(farm_gene_tsp.run_and_wait_end() < 0)
  {
    ff::error("running farm");
    return;
  }
  //ff::ffTime(ff::STOP_TIME);
  //std::cout << "Time: " << ff::ffTime(ff::GET_TIME) << "\n";
  population_ = std::move(*shared_population);
  fitness_ = std::move(*shared_fitness);
  global_best_ = std::move(*shared_optimum);
  return;
  }

  BestSolution get_current_optimum() const { return global_best_; }

private:
  size_t num_workers;
  size_t chunks_size; // number of chromosome that each worker have to deal with

  void init_population()
  {
    size_t i;
    population_.reserve(config_.population_size);
    for(i = 0; i < config_.population_size; ++i)
    {
      Tour chromosome(config_.chromosome_size);
      std::iota(chromosome.begin(), chromosome.end(), 0);
      std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});
      population_.emplace_back(chromosome);
    }
  }
};

#endif // GENETIC_TSP_FF_H