#ifndef GENETIC_TSP_PAR_POOL_H
#define GENETIC_TSP_PAR_POOL_H

#include "domain.hpp"
#include "executors/thread_pool_executor.hpp"
#include "genetic_tsp.hpp"
#include "random_context.hpp"

#include <cstddef>
#include <utility>

class Genetic_TSP_Parallel_Pool
{
public:
  Genetic_TSP_Parallel_Pool(std::size_t worker_count,
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
  ThreadPoolExecutor executor_;
};

#endif // GENETIC_TSP_PAR_POOL_H
