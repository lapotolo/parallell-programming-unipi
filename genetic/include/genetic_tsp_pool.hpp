#ifndef GENETIC_TSP_PAR_POOL_H
#define GENETIC_TSP_PAR_POOL_H

#include "genetic.hpp"
#include "mutation.hpp"
#include "partition.hpp"
#include "pool.hpp"

#include <thread>

class Genetic_TSP_Parallel_Pool : Genetic_Algorithm<std::vector<std::vector<int>>, std::vector<int>, Fitness>
{
public:
  // constructor. First generation is composed of random (feasible) chromosomes
  Genetic_TSP_Parallel_Pool( size_t nw
                           , size_t max_its  
                           , size_t pop_s // chromosome number
                           , size_t chromo_s
                           , std::function<Fitness(std::vector<int> const&)> f
                           )
                           : num_workers(nw)
                                , curr_glob_opt_idx(0)
                           , my_pool(nw) // the pool call its method start() here!
                           , Genetic_Algorithm(max_its, pop_s, chromo_s,f)

  {
    init_population();
    chromosomes_fitness.resize(pop_s);
    evaluate_population(0, pop_s);
    curr_glob_opt_idx = initialize_current_optimum();
    init_ranges();  // setup ranges for thread tasks' splitting
  }

  void run()
  {
    for(size_t epoch = 0; epoch < max_epochs; ++epoch)
      next_generation();
  }

  std::pair<Fitness, std::vector<int>> get_current_optimum() { return current_optimum; }

private:
  std::vector<std::thread> workers;
  
  size_t num_workers;
  size_t curr_glob_opt_idx; // index of the global optimum in the current population

  Thread_Pool my_pool;
  std::vector<std::pair<size_t, size_t>> ranges;
  std::vector<std::pair<size_t, size_t>> pair_ranges;



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

  void init_ranges()
  {
    // Half-open, non-overlapping ranges covering the complete population.
    ranges = partition_evenly(population_size, num_workers);
    pair_ranges = partition_evenly(population_size / 2, num_workers);
  }

  void next_generation()
  {
    std::vector<std::future<void>> completed_tasks;
    completed_tasks.reserve(std::max(ranges.size(), pair_ranges.size()));

    for(const auto& range : pair_ranges)
    {
      const auto first_pair = range.first;
      const auto last_pair = range.second;
      completed_tasks.push_back(my_pool.enqueue([this, first_pair, last_pair]
        {
          crossover(first_pair, last_pair);
        }));
    }
    for(auto& task : completed_tasks) task.get();

    completed_tasks.clear();
    for(const auto& range : ranges)
    {
      const auto first = range.first;
      const auto last = range.second;
      completed_tasks.push_back(my_pool.enqueue([this, first, last]
        {
          mutate(first, last);
          evaluate_population(first, last);
        }));
    }
    for(auto& task : completed_tasks) task.get();

    selection(0, population_size);
  }

  void crossover(size_t const& pair_s, size_t const& pair_e)
  {
    size_t i, j, left, right;

    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

    std::discrete_distribution<> biased_coin({ 1-CROSSOVER_PROB, CROSSOVER_PROB });
  
    for(size_t pair_idx = pair_s; pair_idx < pair_e; ++pair_idx)
    {
      i = 2 * pair_idx;
      if(biased_coin(gen))
      {
        std::uniform_int_distribution<> left_distr(1, ((chromosome_size)/2)-1);
        std::uniform_int_distribution<> right_distr(chromosome_size/2, chromosome_size-2);
        left  = left_distr(gen);
        right = right_distr(gen);

        // setup the structures to build in the end two feasible offspings
        std::deque<int> tmp_chromo, missing;
        std::vector<int> counter_1(chromosome_size, 0), counter_2(chromosome_size, 0);

        for(j = left; j <= right; ++j) tmp_chromo.push_back(population[i][j]);        
        // copy central part of second parent into the central part of the first parent
        for(j = left; j <= right; ++j) population[i][j] = population[i+1][j];
        // viceversa, copy central part of first parent into the central part of the second parent
        for(j = left; j <= right; ++j) { population[i+1][j] = tmp_chromo.front(); tmp_chromo.pop_front(); }

        // SANITIZE PHASE
        // count number of occurrences for each symbol in both the two new offsprings
        for(j = 0; j < chromosome_size; ++j) { counter_1[population[i][j]]++; counter_2[population[i+1][j]]++; }
        
        // use a deque to keep track of missing numbers of the first offspring on the front
        // and missing numbers of the second offspring in the back
        for(j = 0; j < chromosome_size; ++j) { if(counter_1[j] == 0 ) missing.push_front(j); if(counter_2[j] == 0 ) missing.push_back(j); }
        if(missing.size())
        {
          // replace doubles entries with the ones in missing
          for(j = 0; j < chromosome_size; ++j)
          {
            if(counter_1[population[i][j]] == 2)
            {
              counter_1[population[i][j]]--;
              counter_1[missing.front()]++;
              population[i][j] = missing.front();
              missing.pop_front();
            }
            if(counter_2[population[i+1][j]] == 2)
            {
              counter_2[population[i+1][j]]--;
              counter_2[missing.back()]++;
              population[i+1][j] = missing.back();
              missing.pop_back();
            }
          }
        } // end if(missing.size())
      } // end if(biased_cpid)
    } //end for(chunk...)
  }

  // here the mutation is a simple swap of two elements of the chromosome
  void mutate(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;
    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

    std::discrete_distribution<> biased_coin({ 1-MUTATION_PROB, MUTATION_PROB });

    for(i=chunk_s; i < chunk_e; ++i)
      if( i != curr_glob_opt_idx and biased_coin(gen))
      {
        const auto positions = draw_distinct_indices(chromosome_size, gen);
        std::swap(population[i][positions.first], population[i][positions.second]);
      }
  }

  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;

    for(i=chunk_s; i < chunk_e; ++i)
    { // putting emplace_back here instead on assignment operator yields a "double free or corruption (!prev)"
      chromosomes_fitness[i] = fit_fun(population[i]); // O(m) part
    }
  }

  void selection(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;
    auto curr_gen_min_idx = chunk_s;
    auto curr_gen_max_idx = chunk_s;

    auto curr_gen_min_val = current_optimum.first;
    auto curr_gen_max_val = curr_gen_min_val;
    for(i=chunk_s; i < chunk_e; ++i)
    {
      // check if we have a new minimum for the current generation 
      if(chromosomes_fitness[i] < curr_gen_min_val)
      {
        curr_gen_min_idx = i;
        curr_gen_min_val = chromosomes_fitness[i];
      }
      // check if we have a new maximum for the current generation 
      else if(chromosomes_fitness[i] > curr_gen_max_val)
      {
        curr_gen_max_idx = i;
        curr_gen_max_val = chromosomes_fitness[i];
      }
    }
    // if in this generation we found a new optimum
    // then we record it in the proper a class field
    if(curr_gen_min_val < current_optimum.first)
    {
      current_optimum = std::make_pair(curr_gen_min_val, population[curr_gen_min_idx]);
      curr_glob_opt_idx = curr_gen_min_idx;
    }
    // inject the global optimum from previous generations in the current generation
    // in place of the worst chromosome of the current generation    
    chromosomes_fitness[curr_gen_max_idx] = current_optimum.first;
    population[curr_gen_max_idx]          = current_optimum.second;
    curr_glob_opt_idx                     = curr_gen_max_idx;
  }


};

#endif // GENETIC_TSP_PAR_H
