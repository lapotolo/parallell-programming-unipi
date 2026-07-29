#ifndef POOL_H
#define POOL_H

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class Thread_Pool
{
public:
  using Task = std::function<void()>;

  explicit Thread_Pool(std::size_t worker_count)
  {
    if(worker_count == 0)
      throw std::invalid_argument{"worker_count must be greater than zero"};
    start(worker_count);
  }

  ~Thread_Pool()
  {
    stop();
  }

  Thread_Pool(const Thread_Pool&) = delete;
  Thread_Pool& operator=(const Thread_Pool&) = delete;
  Thread_Pool(Thread_Pool&&) = delete;
  Thread_Pool& operator=(Thread_Pool&&) = delete;

  template<typename Function>
  auto enqueue(Function&& function)
    -> std::future<std::invoke_result_t<std::decay_t<Function>&>>
  {
    using Callable = std::decay_t<Function>;
    using Result = std::invoke_result_t<Callable&>;

    auto packaged = std::make_shared<std::packaged_task<Result()>>(
      std::forward<Function>(function));
    auto future = packaged->get_future();

    {
      std::lock_guard<std::mutex> lock{mutex_};
      if(stopping_)
        throw std::runtime_error{"cannot enqueue a task after pool shutdown"};

      tasks_.emplace([packaged] {
        (*packaged)();
      });
    }

    condition_.notify_one();
    return future;
  }

private:
  std::vector<std::thread> workers_;
  std::condition_variable condition_;
  std::mutex mutex_;
  bool stopping_ = false;
  std::queue<Task> tasks_;

  void start(std::size_t worker_count)
  {
    workers_.reserve(worker_count);
    try
    {
      for(std::size_t worker = 0; worker < worker_count; ++worker)
      {
        workers_.emplace_back([this] {
          worker_loop();
        });
      }
    }
    catch(...)
    {
      stop();
      throw;
    }
  }

  void worker_loop()
  {
    for(;;)
    {
      Task task;
      {
        std::unique_lock<std::mutex> lock{mutex_};
        condition_.wait(lock, [this] {
          return stopping_ || !tasks_.empty();
        });

        if(stopping_ && tasks_.empty()) return;

        task = std::move(tasks_.front());
        tasks_.pop();
      }

      task();
    }
  }

  void stop() noexcept
  {
    {
      std::lock_guard<std::mutex> lock{mutex_};
      stopping_ = true;
    }

    condition_.notify_all();
    for(auto& worker : workers_)
    {
      if(worker.joinable()) worker.join();
    }
  }
};

#endif // POOL_H
