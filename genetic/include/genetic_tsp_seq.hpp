#ifndef GENETIC_TSP_SEQ_H
#define GENETIC_TSP_SEQ_H

#include "genetic.hpp"

class Genetic_TSP_Sequential : Genetic_Algorithm<std::vector<std::vector<int>>, std::vector<int>, int32_t>
{
public:
  // constructor,
  Genetic_TSP_Sequential( size_t max_its
                        , size_t pop_s // chromosome number
                        , size_t chromo_s
                        , float p1
                        , float p2
                        , std::function<int32_t(std::vector<int> const&)> f
                        )
                        : curr_glob_opt_idx(0)
                        , Genetic_Algorithm(max_its, pop_s, chromo_s, p1, p2 ,f)
  {
    init_population();
    chromosomes_fitness.reserve(pop_s);
    evaluate_population(0, pop_s);
    current_optimum = std::make_pair( f(population[curr_glob_opt_idx])
                                    , population[curr_glob_opt_idx]);
  }

  void run()
  {
    size_t curr_epoch = 0;
    while( curr_epoch++ < max_epochs)
      next_generation();
  }

  std::pair<int32_t, std::vector<int>> get_current_optimum() { return current_optimum; }   

private:
  size_t curr_glob_opt_idx; // index of the global optimum in the current population
  
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

  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;

    for(i=chunk_s; i < chunk_e; ++i) // CHECK THIS LOOP IF SOMETHING WRONG
    {
      chromosomes_fitness.emplace_back(fit_fun(population[i])); // O(m) part
    }
  }
  
  void next_generation()
  {
    crossover(0, population_size);
    mutate(0, population_size);
    evaluate_population(0, population_size);
    selection(0, population_size);
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

    // inject the global optimum in the current generation
    // in place of the worst chromosome of the current generation 
    chromosomes_fitness[curr_gen_max_idx] = current_optimum.first;
    population[curr_gen_max_idx]          = current_optimum.second;
    curr_glob_opt_idx                     = curr_gen_max_idx;
    
  }

  void crossover(size_t const& chunk_s, size_t const& chunk_e) // recall, index chunk_e is not in the computed interval
  {
    size_t i, j, left, right;

    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

    std::discrete_distribution<> biased_coin({ 1-crossover_prob, crossover_prob });
  
    for(i=chunk_s; i < chunk_e-1; i+=2)
    {
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

    std::discrete_distribution<> biased_coin({ 1-mutation_prob, mutation_prob });
    std::uniform_int_distribution<> idx_distr(0, chromosome_size-1);

    for(i=chunk_s; i < chunk_e; ++i)
      if( i != curr_glob_opt_idx and biased_coin(gen))
        std::swap(population[i][idx_distr(gen)], population[i][idx_distr(gen)]); // thread safe?
  }


};

#endif // GENETIC_TSP_SEQ_H