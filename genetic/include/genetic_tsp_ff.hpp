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
                , size_t pop_s // chromosome number
                , size_t chromo_s
                , float p1
                , float p2
                , std::function<int32_t(std::vector<int> const&)> f
                )
                : num_workers(nw)
                , curr_glob_opt_idx(0)

                , Genetic_Algorithm(max_its, pop_s, chromo_s, p1, p2 ,f)

  {
    //chunks_size = pop_s/nw;
    init_population();
    chromosomes_fitness.reserve(pop_s);
    //evaluate_population(0, pop_s);  CHECK IF NEEDED
    current_optimum = std::make_pair( f(population[curr_glob_opt_idx])
                                    ,   population[curr_glob_opt_idx]);
  }

  void run();

  std::pair<int32_t, std::vector<int>> get_current_optimum() { return current_optimum; }

private:
  friend class TSP_Worker;
  friend class TSP_Master;

  size_t num_workers;
  size_t chunks_size; // number of chromosome that each worker have to deal with
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

  TSP_Task evaluate_population(size_t const& chunk_s, size_t const& chunk_e)
  {
    auto sub_pop_min_idx = chunk_s;
    auto sub_pop_max_idx = chunk_s;

    auto sub_pop_min_val = fit_fun(population[0]);
    auto sub_pop_max_val = sub_pop_min_val;
    
    size_t i;
    for(i=chunk_s; i < chunk_e; ++i)
    { 
      chromosomes_fitness[i] = fit_fun(population[i]); // O(m) part
      // looking for new best individual
      if(chromosomes_fitness[i] < sub_pop_min_val)
      {
        sub_pop_min_val = chromosomes_fitness[i];
        sub_pop_min_idx = i;
      }
      // looking for new worst individual
      if(chromosomes_fitness[i] > sub_pop_max_val)
      {
        sub_pop_max_val = chromosomes_fitness[i];
        sub_pop_max_idx = i;
      }
    }
    return TSP_Task{sub_pop_min_idx, sub_pop_max_idx};
  }
  
  void selection(std::vector<TSP_Task> const& workers_results)
  {
    auto curr_gen_min_idx = workers_results[0].fst_idx;
    auto curr_gen_max_idx = workers_results[0].snd_idx;

    auto curr_gen_min_val = current_optimum.first;
    auto curr_gen_max_val = curr_gen_min_val;
    
    for(auto const& t : workers_results)
    {
      // check if we have a new minimum for the current generation 
      if(chromosomes_fitness[t.fst_idx] < curr_gen_min_val)
      {
        curr_gen_min_idx = t.fst_idx;
        curr_gen_min_val = chromosomes_fitness[t.fst_idx];
      }
      // check if we have a new maximum for the current generation 
      else if(chromosomes_fitness[t.snd_idx] > curr_gen_max_val)
      {
        curr_gen_max_idx = t.snd_idx;
        curr_gen_max_val = chromosomes_fitness[t.snd_idx];
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
  // ************************************************************************
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

#endif // GENETIC_TSP_FF_H