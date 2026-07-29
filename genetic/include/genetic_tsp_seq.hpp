#ifndef GENETIC_TSP_SEQ_H
#define GENETIC_TSP_SEQ_H

#include "genetic.hpp"
#include "mutation.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

class Genetic_TSP_Sequential : Genetic_Algorithm
{
public:
  // constructor,
  Genetic_TSP_Sequential(GeneticConfig config, FitnessFunction fitness_function)
                        : Genetic_Algorithm(std::move(config), std::move(fitness_function))
                        , curr_glob_opt_idx(0)
  {
    init_population();
    state_.fitness.resize(config_.population_size);
    evaluate_population(0, config_.population_size);
    curr_glob_opt_idx = initialize_current_optimum();
  }

  void run()
  {
    for(size_t epoch = 0; epoch < config_.epochs; ++epoch)
      next_generation();
  }

  BestSolution get_current_optimum() const { return state_.global_best; }

private:
  size_t curr_glob_opt_idx; // index of the global optimum in the current population

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

  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;

    for(i=chunk_s; i < chunk_e; ++i) // CHECK THIS LOOP IF SOMETHING WRONG
    {
      state_.fitness[i] = fitness_function_(state_.population[i]); // O(m) part
    }
  }

  void next_generation()
  {
    crossover(0, config_.population_size / 2);
    mutate(0, config_.population_size);
    evaluate_population(0, config_.population_size);
    selection(0, config_.population_size);
  }

  void selection(size_t const& chunk_s, size_t const& chunk_e)
  {
    size_t i;
    auto curr_gen_min_idx = chunk_s;
    auto curr_gen_max_idx = chunk_s;

    auto curr_min_value = state_.global_best.fitness;
    auto curr_max_value = curr_min_value;
    for(i=chunk_s; i < chunk_e; ++i) // CHECK THIS LOOP IF SOMETHING WRONG
    {
      // check if we have a new minimum for the current generation
      if(state_.fitness[i] < curr_min_value)
      {
        curr_gen_min_idx = i;
        curr_min_value   = state_.fitness[i];
      }
      else if(state_.fitness[i] > curr_max_value)
      {
        curr_gen_max_idx = i;
        curr_max_value = state_.fitness[i];
      }
    }
    // if in this generation we found  a new optimum
    // then we record it in the proper a class field
    if(curr_min_value < state_.global_best.fitness)
    {
      state_.global_best = BestSolution{curr_min_value, state_.population[curr_gen_min_idx]};
      curr_glob_opt_idx = curr_gen_min_idx;
    }

    // inject the global optimum in the current generation
    // in place of the worst chromosome of the current generation
    state_.fitness[curr_gen_max_idx] = state_.global_best.fitness;
    state_.population[curr_gen_max_idx]          = state_.global_best.tour;
    curr_glob_opt_idx                     = curr_gen_max_idx;

  }

  void crossover(size_t const& pair_s, size_t const& pair_e)
  {
    size_t i, j, left, right;

    std::random_device rd;  // get a seed for the random number engine
    std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

    std::discrete_distribution<> biased_coin({ 1-config_.crossover_probability, config_.crossover_probability });

    for(size_t pair_idx = pair_s; pair_idx < pair_e; ++pair_idx)
    {
      i = 2 * pair_idx;
      if(biased_coin(gen))
      {
        std::uniform_int_distribution<> left_distr(1, ((config_.chromosome_size)/2)-1);
        std::uniform_int_distribution<> right_distr(config_.chromosome_size/2, config_.chromosome_size-2);
        left  = left_distr(gen);
        right = right_distr(gen);

        // setup the structures to build in the end two feasible offspings
        std::deque<int> tmp_chromo, missing;
        std::vector<int> counter_1(config_.chromosome_size, 0), counter_2(config_.chromosome_size, 0);

        for(j = left; j <= right; ++j) tmp_chromo.push_back(state_.population[i][j]);
        // copy central part of second parent into the central part of the first parent
        for(j = left; j <= right; ++j) state_.population[i][j] = state_.population[i+1][j];
        // viceversa, copy central part of first parent into the central part of the second parent
        for(j = left; j <= right; ++j) { state_.population[i+1][j] = tmp_chromo.front(); tmp_chromo.pop_front(); }

        // SANITIZE PHASE
        // count number of occurrences for each symbol in both the two new offsprings
        for(j = 0; j < config_.chromosome_size; ++j) { counter_1[state_.population[i][j]]++; counter_2[state_.population[i+1][j]]++; }

        // use a deque to keep track of missing numbers of the first offspring on the front
        // and missing numbers of the second offspring in the back
        for(j = 0; j < config_.chromosome_size; ++j) { if(counter_1[j] == 0 ) missing.push_front(j); if(counter_2[j] == 0 ) missing.push_back(j); }
        if(missing.size())
        {
          // replace doubles entries with the ones in missing
          for(j = 0; j < config_.chromosome_size; ++j)
          {
            if(counter_1[state_.population[i][j]] == 2)
            {
              counter_1[state_.population[i][j]]--;
              counter_1[missing.front()]++;
              state_.population[i][j] = missing.front();
              missing.pop_front();
            }
            if(counter_2[state_.population[i+1][j]] == 2)
            {
              counter_2[state_.population[i+1][j]]--;
              counter_2[missing.back()]++;
              state_.population[i+1][j] = missing.back();
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

    std::discrete_distribution<> biased_coin({ 1-config_.mutation_probability, config_.mutation_probability });

    for(i=chunk_s; i < chunk_e; ++i)
      if( i != curr_glob_opt_idx and biased_coin(gen))
      {
        const auto positions = draw_distinct_indices(config_.chromosome_size, gen);
        std::swap(state_.population[i][positions.first], state_.population[i][positions.second]);
      }
  }


};

#endif // GENETIC_TSP_SEQ_H
