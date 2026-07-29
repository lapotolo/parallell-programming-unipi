#include "benchmark_application.hpp"
#include "cli.hpp"
#include "executors/sequential_executor.hpp"

#include <cstddef>
#include <iostream>

int main(int argc, const char* argv[])
{
  if(argc != 4 && argc != 5)
  {
    std::cout
      << "Sequential Genetic TSP Usage is: "
      << "<max_epochs> <population_size> <chromosome_size> [seed]\n";
    return -1;
  }

  std::size_t epochs = 0;
  std::size_t population_size = 0;
  std::size_t chromosome_size = 0;
  if(!parse_size_argument(argv[1], epochs) ||
     !parse_size_argument(argv[2], population_size) ||
     !parse_size_argument(argv[3], chromosome_size) ||
     !validate_common_configuration(population_size, chromosome_size))
  {
    return -1;
  }

  RandomSeed seed = make_random_seed();
  if(argc == 5 && !parse_seed_argument(argv[4], seed))
  {
    std::cerr << "invalid seed.\n";
    return -1;
  }

  const GeneticConfig config{population_size, chromosome_size, epochs};
  SequentialExecutor executor;
  return run_tsp_benchmark(
    config,
    seed,
    executor,
    BenchmarkVariant{"seq", "t_seq", std::nullopt});
}
