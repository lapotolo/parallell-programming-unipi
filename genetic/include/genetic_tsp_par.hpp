#ifndef GENETIC_TSP_PAR_H
#define GENETIC_TSP_PAR_H

#include <thread>

#include "genetic.hpp"
//#include "thread_pool.hpp"

class Genetic_TSP_Parallel : Genetic_Algorithm<std::vector<std::vector<int>>, std::vector<int>, int32_t>
{
public:
  // constructor. First generation is composed of random (feasible) chromosomes
  Genetic_TSP_Parallel( size_t nw
                      , size_t pop_s // chromosome number
                      , size_t chromo_s
                      , float p1
                      , float p2
                      , std::function<int32_t(std::vector<int> const&)> f
                      )
                      : num_workers(nw)
                      , Genetic_Algorithm(pop_s, chromo_s, p1, p2 ,f)

  {
    //workers.resize(nw);
    init_population();
    chromosomes_fitness.resize(pop_s);
    curr_glob_opt_idx = 0;
    current_optimum = std::make_pair( f(population[curr_glob_opt_idx])
                                    , population[curr_glob_opt_idx]);
    evaluate_population(0, pop_s);
  }

  void next_generation()
  {
    size_t i;
    size_t chunk_size { population_size / num_workers }; // number of chromosome that each worker have to deal with

    std::vector<std::pair<size_t, size_t>> ranges;

    // setup ranges to be given to the workers to work without data races
    for(i=0; i<num_workers; i++)
      ranges.push_back(std::make_pair( i*chunk_size
                                     ,(i != (num_workers-1) ? (i+1)*chunk_size : population_size)));
    


    // PARALLEL FORK/JOIN MODEL TO APPLY CROSSOVERS TO CHROMOSOMES
    for(i = 0; i < num_workers; ++i)
      workers.push_back(std::move(std::thread( &Genetic_TSP_Parallel::crossover
                                             , this
                                             , ranges[i].first
                                             , ranges[i].second)));  // FORK num_workers threads
    for(auto & thr : workers)
      thr.join(); // JOIN: u cant proceed in the computation unless every spawned thread completed its task

    workers.clear();
    
    // PARALLEL FORK/JOIN MODEL TO APPLY MUTATION TO CHROMOSOMES
    for(i = 0; i < num_workers; ++i)
      workers.push_back(std::move(std::thread( &Genetic_TSP_Parallel::mutate
                                             , this
                                             , ranges[i].first
                                             , ranges[i].second)));  // FORK num_workers threads
    for(auto & thr : workers)
      thr.join(); // JOIN

    workers.clear();

    // PARALLEL FORK/JOIN MODEL FOR CHROMOSOMES FITNESS EVALUATION
    for(i = 0; i < num_workers; ++i)
      workers.push_back(std::move(std::thread( &Genetic_TSP_Parallel::evaluate_population
                                             , this
                                             , ranges[i].first
                                             , ranges[i].second)));  // FORK num_workers threads
    for(auto & thr : workers)
      thr.join(); // JOIN

    workers.clear();

    selection(0, population_size);
  }

  std::pair<int32_t, std::vector<int>> get_current_optimum() { return current_optimum; }

private:
  size_t num_workers;
  size_t curr_glob_opt_idx; // index of the global optimum in the current population
  std::vector<std::thread> workers;
  
  void init_population()
  {  
    size_t i;  
    population.reserve(population_size);
    for(i = 0; i < population_size; ++i)
    {
      std::vector<int> chromosome(chromosome_size);
      std::iota(chromosome.begin(), chromosome.end(), 0);
      std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});
      population.push_back(chromosome);
    }
  }


  //apply the crossover reproduction with a given probability
  // i.e. if parent 1 is ----------
  //         parent 2 is ~~~~~~~~~~
  // we get two sons:    ---~~~~---
  //                     ~~~----~~~
  void crossover(size_t const& chunk_s, size_t const& chunk_e) // recall, index chunk_e is not in the computed interval
  {
    size_t i, j, left, right;

    std::vector<int> tmp_chromo_1(chromosome_size);
    std::vector<int> tmp_chromo_2(chromosome_size);

    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()
  
    std::discrete_distribution<> biased_coin({ 1-crossover_prob, crossover_prob }); // distribution simulating a biased coin that gives 1 with prob=crossover_prob

    std::uniform_int_distribution<> left_idx_distr(1, ((chromosome_size-1)>>1)-1); // generate a random index between [1, chromosome.size/2] ie. cannot generate the first index
    std::uniform_int_distribution<> right_idx_distr(((chromosome_size-1)>>1) + 1, chromosome_size-2); // generate a random index between [chromosome.size/2, chromosome.size-2] ie. cannot generate the last index
  
    for(i=chunk_s; i < chunk_e; i+=2)
    {
      if(biased_coin(gen))
      {
        tmp_chromo_1 = population[i];
        tmp_chromo_2 = population[i+1];
        left  = left_idx_distr(gen);
        right = right_idx_distr(gen);

        // copy central part of second parent into the central part of the first parent
        for(j = left; j <= right; ++j) population[i][j] = tmp_chromo_2[j];

        // viceversa, copy central part of first parent into the central part of the second parent
        for(j = left; j <= right; ++j) population[i+1][j] = tmp_chromo_1[j];
      }
    }
  }


  // here the mutation is a simple swap of two elements of the chromosome
  void mutate(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;
    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

    std::discrete_distribution<> biased_coin({ 1-mutation_prob, mutation_prob });
    std::uniform_int_distribution<> idx_distr(0, chromosome_size-1);

    for(i=chunk_s; i < chunk_e; ++i)
      if( i != curr_glob_opt_idx and biased_coin(gen))
        std::swap(population[i][idx_distr(gen)], population[i][idx_distr(gen)]); // thread safe?
  }

  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;

    for(i=chunk_s; i < chunk_e; ++i) // CHECK THIS LOOP IF SOMETHING WRONG
    {
      chromosomes_fitness[i] = fit_fun(population[i]); // O(m) part
    }
  }

  void selection(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;
    auto curr_gen_min_idx = chunk_s;
    auto curr_gen_max_idx = chunk_s;

    auto curr_min_value = current_optimum.first;
    auto curr_max_value = curr_min_value;
    for(i=chunk_s; i < chunk_e; ++i) // CHECK THIS LOOP IF SOMETHING WRONG
    {
      // check if we have a new minimum for the current generation 
      if(chromosomes_fitness[i] < curr_min_value)
      {
        curr_gen_min_idx = i;
        curr_min_value   = chromosomes_fitness[i];
      }
      else if(chromosomes_fitness[i] > curr_max_value)
      {
        curr_gen_max_idx = i;
        curr_max_value = chromosomes_fitness[i];
      }
    }
    // if in this generation we found  a new optimum
    // then we record it in the proper a class field
    if(curr_min_value < current_optimum.first)
    {
      current_optimum = std::make_pair(curr_min_value, population[curr_gen_min_idx]);
      curr_glob_opt_idx = curr_gen_min_idx;
    }

    // otherwise if this generation does not contain a new optimum
    // inject the global optimum from previous generations in the current generation
    // in place of the worst chromosome of the current generation
    else
    {
      chromosomes_fitness[curr_gen_max_idx] = current_optimum.first;
      population[curr_gen_max_idx]          = current_optimum.second;
      curr_glob_opt_idx                     = curr_gen_max_idx;
    }
  }

};

#endif // GENETIC_TSP_PAR_H