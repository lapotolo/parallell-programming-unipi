#ifndef EXECUTORS_SEQUENTIAL_EXECUTOR_H
#define EXECUTORS_SEQUENTIAL_EXECUTOR_H

#include "executor.hpp"

#include <cstddef>
#include <utility>

class SequentialExecutor
{
public:
  template<typename Function>
  void for_each_range(std::size_t item_count, Function&& function)
  {
    if(item_count == 0) return;
    std::forward<Function>(function)(0, item_count, WorkerId{0});
  }
};

#endif // EXECUTORS_SEQUENTIAL_EXECUTOR_H
