#ifndef EXECUTORS_FASTFLOW_EXECUTOR_H
#define EXECUTORS_FASTFLOW_EXECUTOR_H

#include "executor.hpp"
#include "../partition.hpp"

#include <ff/parallel_for.hpp>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

class FastFlowExecutor
{
public:
  explicit FastFlowExecutor(std::size_t worker_count)
    : worker_count_{validate_worker_count(worker_count)}
    , parallel_for_{static_cast<long>(worker_count_)}
  {
  }

  FastFlowExecutor(const FastFlowExecutor&) = delete;
  FastFlowExecutor& operator=(const FastFlowExecutor&) = delete;
  FastFlowExecutor(FastFlowExecutor&&) = delete;
  FastFlowExecutor& operator=(FastFlowExecutor&&) = delete;

  template<typename Function>
  void for_each_range(std::size_t item_count, Function&& function)
  {
    const auto ranges = partition_evenly(item_count, worker_count_);
    if(ranges.empty()) return;
    if(ranges.size() > static_cast<std::size_t>(std::numeric_limits<long>::max()))
      throw std::length_error{"too many FastFlow ranges"};

    std::vector<std::exception_ptr> errors(ranges.size());
    const auto range_count = static_cast<long>(ranges.size());
    const auto active_workers = static_cast<long>(
      std::min(worker_count_, ranges.size()));

    parallel_for_.parallel_for(
      0,
      range_count,
      [&](long range_index) {
        const auto index = static_cast<std::size_t>(range_index);
        const auto range = ranges[index];
        try
        {
          function(range.first, range.second, index);
        }
        catch(...)
        {
          errors[index] = std::current_exception();
        }
      },
      active_workers);

    for(const auto& error : errors)
    {
      if(error) std::rethrow_exception(error);
    }
  }

  [[nodiscard]] std::size_t worker_count() const noexcept
  {
    return worker_count_;
  }

private:
  static std::size_t validate_worker_count(std::size_t worker_count)
  {
    if(worker_count == 0)
      throw std::invalid_argument{"worker_count must be greater than zero"};
    if(worker_count > static_cast<std::size_t>(std::numeric_limits<long>::max()))
      throw std::length_error{"worker_count does not fit FastFlow's index type"};
    return worker_count;
  }

  std::size_t worker_count_;
  ff::ParallelFor parallel_for_;
};

#endif // EXECUTORS_FASTFLOW_EXECUTOR_H
