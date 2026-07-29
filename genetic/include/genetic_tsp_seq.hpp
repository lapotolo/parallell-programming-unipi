#ifndef GENETIC_TSP_SEQ_H
#define GENETIC_TSP_SEQ_H

#include "domain.hpp"
#include "executors/sequential_executor.hpp"
#include "genetic_tsp.hpp"

#include <utility>

class Genetic_TSP_Sequential
{
public:
  Genetic_TSP_Sequential(GeneticConfig config,
                         FitnessFunction fitness_function)
    : algorithm_{std::move(config), std::move(fitness_function)}
  {
  }

  void run()
  {
    algorithm_.run(executor_);
  }

  [[nodiscard]] BestSolution get_current_optimum() const
  {
    return algorithm_.best_solution();
  }

private:
  GeneticTsp algorithm_;
  SequentialExecutor executor_;
};

#endif // GENETIC_TSP_SEQ_H
