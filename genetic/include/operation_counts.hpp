#ifndef OPERATION_COUNTS_H
#define OPERATION_COUNTS_H

#include <cstddef>

struct OperationCounts
{
  std::size_t generations{};
  std::size_t crossover_pairs_processed{};
  std::size_t mutation_candidates_processed{};
  std::size_t fitness_evaluations{};
};

inline bool operator==(const OperationCounts& first,
                       const OperationCounts& second)
{
  return first.generations == second.generations &&
         first.crossover_pairs_processed ==
           second.crossover_pairs_processed &&
         first.mutation_candidates_processed ==
           second.mutation_candidates_processed &&
         first.fitness_evaluations == second.fitness_evaluations;
}

inline bool operator!=(const OperationCounts& first,
                       const OperationCounts& second)
{
  return !(first == second);
}

#endif // OPERATION_COUNTS_H
