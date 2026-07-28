#ifndef PARTITION_H
#define PARTITION_H

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

using Work_Range = std::pair<std::size_t, std::size_t>;

inline std::vector<Work_Range> partition_evenly(std::size_t element_count,
                                                std::size_t requested_workers)
{
  if(requested_workers == 0)
    throw std::invalid_argument("requested_workers must be greater than zero");

  if(element_count == 0) return {};

  const auto worker_count = std::min(element_count, requested_workers);
  const auto base_size = element_count / worker_count;
  const auto remainder = element_count % worker_count;

  std::vector<Work_Range> ranges;
  ranges.reserve(worker_count);

  std::size_t first = 0;
  for(std::size_t worker = 0; worker < worker_count; ++worker)
  {
    const auto range_size = base_size + (worker < remainder ? 1 : 0);
    const auto last = first + range_size;
    ranges.emplace_back(first, last);
    first = last;
  }

  return ranges;
}


struct Genetic_Work_Range
{
  Work_Range chromosomes;
  Work_Range pairs;
};

inline std::vector<Genetic_Work_Range> partition_crossover_aligned(
  std::size_t population_size,
  std::size_t requested_workers)
{
  if(requested_workers == 0)
    throw std::invalid_argument("requested_workers must be greater than zero");
  if(population_size == 0) return {};

  const auto pair_count = population_size / 2;
  if(pair_count == 0)
    return {{{0, population_size}, {0, 0}}};

  const auto pair_ranges = partition_evenly(pair_count, requested_workers);
  std::vector<Genetic_Work_Range> result;
  result.reserve(pair_ranges.size());

  for(const auto& pair_range : pair_ranges)
    result.push_back({{2 * pair_range.first, 2 * pair_range.second}, pair_range});

  // The unpaired chromosome of an odd population belongs to the last task.
  if(population_size % 2 != 0)
    result.back().chromosomes.second = population_size;

  return result;
}

#endif // PARTITION_H
