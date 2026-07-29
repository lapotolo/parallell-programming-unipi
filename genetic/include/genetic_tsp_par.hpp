#ifndef GENETIC_TSP_PAR_H
#define GENETIC_TSP_PAR_H

#include "domain.hpp"
#include "executors/raw_thread_executor.hpp"
#include "genetic_tsp.hpp"

#include <cstddef>
#include <utility>

class Genetic_TSP_Parallel
{
public:
  Genetic_TSP_Parallel(std::size_t worker_count,
                       GeneticConfig config,
                       FitnessFunction fitness_function)
    : algorithm_{std::move(config), std::move(fitness_function)}
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
  RawThreadExecutor executor_;
};

#endif // GENETIC_TSP_PAR_H
