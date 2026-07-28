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

#endif // PARTITION_H
