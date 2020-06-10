#ifndef GENETIC_H
#define GENETIC_H

#include "conf.hpp"


// not properly but something like an abstract class
template< typename Population_t       // type of the population. Hopefully an stl container of Chomosomes_t
        , typename Chromosome_t       // type of the chromosome
        , typename Fitness_Fun_tout > // return type of the fitness function
class Genetic_Algorithm
{
public:
  // constructor
  Genetic_Algorithm( size_t max_its
                   , size_t pop_s
                   , size_t chromo_s
                   , float p1
                   , float p2
                   , std::function<int32_t(std::vector<int> const&)> f
                   )
                   : 
                     max_epochs(max_its)
                   , population_size(pop_s)
                   , chromosome_size(chromo_s)
                   , crossover_prob(p1)
                   , mutation_prob(p2)
                   , fit_fun(f)
                   {};

  void run(); 

  // returns the current optimum value
  std::pair<Fitness_Fun_tout, Chromosome_t> get_current_optimum();

protected:
  // constructor parameters
  size_t max_epochs;      // maximum number of iterations of the algorithm
  size_t population_size; // population size.
  size_t chromosome_size; // chromosome size (hopefully it is represented as a stl container)
  float crossover_prob;   // probability that two next chromosomes are crossed over during an iteration of the genetic algorithm
  float mutation_prob;    // probability that a chromosome mutates during an iteration of the genetic algorithm
  
  // other fields
  Population_t population;
  std::function<Fitness_Fun_tout(Chromosome_t const&)> fit_fun;
  std::vector<Fitness_Fun_tout> chromosomes_fitness;
  std::pair<Fitness_Fun_tout, Chromosome_t> current_optimum;

  // helper methods used by interface's functions
  // init the population: ie: allocating memory for the matrix representing the population
  void init_population();

  // evolve the population, that is one iteration of the genetic algorithm
  void next_generation();
  
  // evaluate the population and updates the collection chromosomes fitness
  // returns the index of the best chromosome
  void evaluate_population(size_t const& chunk_s, size_t const& chunk_e);

  // apply the crossover reproduction with a given probability
  void crossover(size_t const& chunk_s, size_t const& chunk_e);

  // apply some mutation to the chromosomes with a given probability
  void mutate(size_t const& chunk_s, size_t const& chunk_e);

  // scan the vector of current fitness for each chromosomes,
  // record the current best in `current_optimum` field
  // replace the worst element of the current gen with the best optimum found so far
  void selection(size_t const& chunk_s, size_t const& chunk_e);
  
};

#endif // GENETIC_H
