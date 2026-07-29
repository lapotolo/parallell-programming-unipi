#ifndef GENETIC_TSP_FF_H
#define GENETIC_TSP_FF_H

#include "domain.hpp"
#include "executors/fastflow_executor.hpp"
#include "genetic_tsp.hpp"
#include "random_context.hpp"

#include <cstddef>
#include <utility>

class Genetic_TSP_FF
{
public:
  Genetic_TSP_FF(std::size_t worker_count,
                 GeneticConfig config,
                 FitnessFunction fitness_function,
                       RandomSeed seed = make_random_seed())
    : algorithm_{std::move(config), std::move(fitness_function), seed}
    , executor_{worker_count}
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
  FastFlowExecutor executor_;
};

#endif // GENETIC_TSP_FF_H
