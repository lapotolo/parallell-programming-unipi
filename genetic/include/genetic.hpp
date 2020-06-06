#ifndef GENETIC_H
#define GENETIC_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <random>
#include <chrono> 

// not properly but something like an abstract class
template< typename Population_t       // type of the population. Hopefully an stl container of Chomosomes_t
        , typename Chromosome_t       // type of the chromosome
        , typename Fitness_Fun_tout > // return type of the fitness function
class Genetic_Algorithm
{
public:
  // constructor
  Genetic_Algorithm( size_t pop_s // chromosome number
                   , size_t chromo_s
                   , float p1
                   , float p2
                   , std::function<int32_t(std::vector<int> const&)> f
                   )
                   : 
                   population_size(pop_s)
                   , chromosome_size(chromo_s)
                   , crossover_prob(p1)
                   , mutation_prob(p2)
                   , fit_fun(f)
                   {};

  // evolve the population, that is one iteration of the genetic algorithm
  // param: num_workers : number of thread to deploy
  void next_generation();

  // returns the current optimum value
  std::pair<Fitness_Fun_tout, Chromosome_t> get_current_optimum();

protected:
  // constructor parameters
  size_t population_size; // population size.
  size_t chromosome_size; // chromosome size (hopefully it is represented as a stl container)
  float crossover_prob;   // probability that two next chromosomes are crossed over during an iteration of the genetic algorithm
  float mutation_prob;    // probability that a chromosome mutates during an iteration of the genetic algorithm
  std::function<Fitness_Fun_tout(Chromosome_t const&)> fit_fun;

  Population_t population;

  std::vector<Fitness_Fun_tout> chromosomes_fitness;

  std::pair<Fitness_Fun_tout, Chromosome_t> current_optimum;
  
  // init the population
  void init_population();

  // apply the crossover reproduction with a given probability
  void crossover(size_t const& chunk_s, size_t const& chunk_e);

  // apply some mutation to the chromosomes with a given probability
  void mutate(size_t const& chunk_s, size_t const& chunk_e);
  
  // evaluate the population and updates the collection chromosomes fitness
  // returns the index of the best chromosome
  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e);

  void selection(size_t const& chunk_s, size_t const& chunk_e);
};

#endif // GENETIC_H
