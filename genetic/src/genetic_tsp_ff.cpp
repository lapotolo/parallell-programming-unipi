#include "benchmark_application.hpp"
#include "cli.hpp"
#include "executors/fastflow_executor.hpp"

#include <cstddef>
#include <iostream>

int main(int argc, const char* argv[])
{
  if(argc != 5 && argc != 6)
  {
    std::cout
      << "FastFlow Genetic TSP Usage is: "
      << "<number_of_workers> <max_epochs> <population_size> "
      << "<chromosome_size> [seed]\n";
    return -1;
  }

  std::size_t worker_count = 0;
  std::size_t epochs = 0;
  std::size_t population_size = 0;
  std::size_t chromosome_size = 0;
  if(!parse_size_argument(argv[1], worker_count) ||
     !parse_size_argument(argv[2], epochs) ||
     !parse_size_argument(argv[3], population_size) ||
     !parse_size_argument(argv[4], chromosome_size) ||
     !validate_parallel_configuration(
       worker_count,
       population_size,
       chromosome_size))
  {
    return -1;
  }

  RandomSeed seed = make_random_seed();
  if(argc == 6 && !parse_seed_argument(argv[5], seed))
  {
    std::cerr << "invalid seed.\n";
    return -1;
  }

  const GeneticConfig config{population_size, chromosome_size, epochs};
  FastFlowExecutor executor{worker_count};
  return run_tsp_benchmark(
    config,
    seed,
    executor,
    BenchmarkVariant{"FF", "t_ff", worker_count});
}
