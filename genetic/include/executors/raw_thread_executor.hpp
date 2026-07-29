#ifndef EXECUTORS_RAW_THREAD_EXECUTOR_H
#define EXECUTORS_RAW_THREAD_EXECUTOR_H

#include "executor.hpp"
#include "../partition.hpp"

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

class RawThreadExecutor
{
public:
  explicit RawThreadExecutor(std::size_t worker_count)
    : worker_count_{worker_count}
  {
    if(worker_count_ == 0)
      throw std::invalid_argument{"worker_count must be greater than zero"};
  }

  RawThreadExecutor(const RawThreadExecutor&) = delete;
  RawThreadExecutor& operator=(const RawThreadExecutor&) = delete;
  RawThreadExecutor(RawThreadExecutor&&) = delete;
  RawThreadExecutor& operator=(RawThreadExecutor&&) = delete;

  template<typename Function>
  void for_each_range(std::size_t item_count, Function&& function)
  {
    const auto ranges = partition_evenly(item_count, worker_count_);
    std::vector<std::thread> threads;
    std::vector<std::exception_ptr> errors(ranges.size());
    threads.reserve(ranges.size());

    const auto join_all = [&threads] {
      for(auto& thread : threads)
      {
        if(thread.joinable()) thread.join();
      }
    };

    try
    {
      for(std::size_t worker = 0; worker < ranges.size(); ++worker)
      {
        const auto range = ranges[worker];
        threads.emplace_back([&, worker, range] {
          try
          {
            function(range.first, range.second, worker);
          }
          catch(...)
          {
            errors[worker] = std::current_exception();
          }
        });
      }
    }
    catch(...)
    {
      join_all();
      throw;
    }

    join_all();
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
  std::size_t worker_count_;
};

#endif // EXECUTORS_RAW_THREAD_EXECUTOR_H
