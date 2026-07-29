#ifndef DOMAIN_H
#define DOMAIN_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

using City = int;
using Tour = std::vector<City>;
using Population = std::vector<Tour>;
using Fitness = std::uint64_t;
using FitnessVector = std::vector<Fitness>;
using FitnessFunction = std::function<Fitness(const Tour&)>;

struct BestSolution
{
  Fitness fitness{};
  Tour tour;
};

inline bool operator==(const BestSolution& first, const BestSolution& second)
{
  return first.fitness == second.fitness && first.tour == second.tour;
}

inline bool operator!=(const BestSolution& first, const BestSolution& second)
{
  return !(first == second);
}

struct GeneticConfig
{
  std::size_t population_size{};
  std::size_t chromosome_size{};
  std::size_t epochs{};
  double crossover_probability{0.5};
  double mutation_probability{0.3};

  void validate() const
  {
    if(population_size == 0)
      throw std::invalid_argument{"population_size must be greater than zero"};
    if(chromosome_size < 4)
      throw std::invalid_argument{"chromosome_size must be at least four"};
    if(crossover_probability < 0.0 || crossover_probability > 1.0)
      throw std::invalid_argument{"crossover_probability must be in [0, 1]"};
    if(mutation_probability < 0.0 || mutation_probability > 1.0)
      throw std::invalid_argument{"mutation_probability must be in [0, 1]"};
  }
};

#endif // DOMAIN_H
