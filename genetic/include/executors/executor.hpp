#ifndef EXECUTORS_EXECUTOR_H
#define EXECUTORS_EXECUTOR_H

#include <cstddef>
#include <utility>

using WorkerId = std::size_t;

// Executors satisfy the following C++17 duck-typed interface:
//
//   executor.for_each_range(item_count, function)
//
// `function` is invoked as:
//
//   function(first, last, worker_id)
//
// where [first, last) is a non-empty range. The call returns only after every
// range has completed, and executor implementations must propagate failures.
template<typename Executor, typename Function>
void execute_ranges(Executor& executor,
                    std::size_t item_count,
                    Function&& function)
{
  executor.for_each_range(
    item_count,
    std::forward<Function>(function));
}

#endif // EXECUTORS_EXECUTOR_H
