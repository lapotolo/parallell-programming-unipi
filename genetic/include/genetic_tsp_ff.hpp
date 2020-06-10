#ifndef FF_POPULATION_H
#define FF_POPULATION_H

#include "genetic.hpp"
#include "ff_farm_tsp.hpp"

// #define DEBUG 

/**********************************************
  FF NOTES:

 nome_pipe.wrap_around() feedback chanels

***********************************************/


class Genetic_TSP_FF : Genetic_Algorithm<std::vector<std::vector<int>>, std::vector<int>, int32_t> 
{
public:
  // constructor. First generation is composed of random (feasible) chromosomes
  Genetic_TSP_FF( size_t nw
                , size_t max_its
                , size_t pop_s // chromosome number
                , size_t chromo_s
                , float p1
                , float p2
                , std::function<int32_t(std::vector<int> const&)> f
                )
                : num_workers(nw)
                , Genetic_Algorithm(max_its, pop_s, chromo_s, p1, p2 ,f)

  {
    curr_glob_opt_idx = 0;
    //chunks_size = pop_s/nw;
    init_population();
    chromosomes_fitness.resize(pop_s);
    current_optimum = std::make_pair( f(population[curr_glob_opt_idx])
                                    , population[curr_glob_opt_idx]);
  }

  void run();

  void merge_opts();

  std::pair<int32_t, std::vector<int>> get_current_optimum() { return current_optimum; }

private:
  size_t curr_glob_opt_idx; // index of the global optimum in the current population
  size_t num_workers;
  std::vector<std::thread> workers;

  size_t chunks_size; // number of chromosome that each worker have to deal with

  // MAYBE NOT NEEDED
  //std::vector<std::pair<size_t, size_t>> ranges;
  //void init_ranges();

  void init_population();
  // void next_generation();
  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e);
  void selection(size_t const& chunk_s, size_t const& chunk_e);
  void crossover(size_t const& chunk_s, size_t const& chunk_e);
  void mutate(size_t const& chunk_s, size_t const& chunk_e);
};

//***************************************************************
//***************************************************************
//***************************************************************
