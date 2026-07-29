#ifndef EXECUTORS_THREAD_POOL_EXECUTOR_H
#define EXECUTORS_THREAD_POOL_EXECUTOR_H

#include "executor.hpp"
#include "../partition.hpp"
#include "../pool.hpp"

#include <cstddef>
#include <future>
#include <stdexcept>
#include <utility>
#include <vector>

class ThreadPoolExecutor
{
public:
  explicit ThreadPoolExecutor(std::size_t worker_count)
    : worker_count_{worker_count}
    , pool_{worker_count}
  {
    if(worker_count_ == 0)
      throw std::invalid_argument{"worker_count must be greater than zero"};
  }

  ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
  ThreadPoolExecutor& operator=(const ThreadPoolExecutor&) = delete;
  ThreadPoolExecutor(ThreadPoolExecutor&&) = delete;
  ThreadPoolExecutor& operator=(ThreadPoolExecutor&&) = delete;

  template<typename Function>
  void for_each_range(std::size_t item_count, Function&& function)
  {
    const auto ranges = partition_evenly(item_count, worker_count_);
    std::vector<std::future<void>> futures;
    futures.reserve(ranges.size());

    for(std::size_t worker = 0; worker < ranges.size(); ++worker)
    {
      const auto range = ranges[worker];
      futures.push_back(pool_.enqueue([&, worker, range] {
        function(range.first, range.second, worker);
      }));
    }

    for(auto& future : futures)
      future.get();
  }

  [[nodiscard]] std::size_t worker_count() const noexcept
  {
    return worker_count_;
  }

private:
  std::size_t worker_count_;
  ThreadPool pool_;
};

#endif // EXECUTORS_THREAD_POOL_EXECUTOR_H
