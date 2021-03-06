#ifndef GENETIC_TSP_FF_H
#define GENETIC_TSP_FF_H

#include "genetic.hpp"
#include "ff_farm_tsp.hpp"




class Genetic_TSP_FF : Genetic_Algorithm<std::vector<std::vector<int>>, std::vector<int>, int32_t> 
{
public:
  // constructor. First generation is composed of random (feasible) chromosomes
  Genetic_TSP_FF( size_t nw
                , size_t max_its
                , size_t pop_s
                , size_t chromo_s
                , std::function<int32_t(std::vector<int> const&)> f
                )
                : num_workers(nw)
                , Genetic_Algorithm(max_its, pop_s, chromo_s, f)
  {
    init_population();
    chromosomes_fitness.resize(pop_s, 0); // WHY IS THIS NEEDED?
    current_optimum = std::make_pair( f(population[0])
                                    ,   population[0]);
  }


  void run() // FF is deployed in here
  {
  size_t i;

  TSP_Master master (num_workers
                   , max_epochs
                   , population_size
                   , std::make_shared<std::vector<std::vector<int>>>(population)
                   , std::make_shared<std::vector<int>>(chromosomes_fitness)
                   , std::make_shared<std::function<int32_t(std::vector<int> const&)>>(fit_fun)
                   , std::make_shared<std::pair<int32_t, std::vector<int>>>(current_optimum)
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
  return;
  }

  std::pair<int32_t, std::vector<int>> get_current_optimum() { return current_optimum; }

private:
  size_t num_workers;
  size_t chunks_size; // number of chromosome that each worker have to deal with

  void init_population()
  {  
    size_t i;  
    population.reserve(population_size);
    for(i = 0; i < population_size; ++i)
    {
      std::vector<int> chromosome(chromosome_size);
      std::iota(chromosome.begin(), chromosome.end(), 0);
      std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});
      population.emplace_back(chromosome);
    }
  }
};

#endif // GENETIC_TSP_FF_H