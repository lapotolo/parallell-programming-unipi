#ifndef POOL_H
#define POOL_H


#include <condition_variable>
#include <functional>
#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <queue>

// https://www.youtube.com/watch?v=eWTGtp3HXiw

class Thread_Pool
{
public:
  using Task = std::function<void()>;

  // CTOR
  explicit Thread_Pool(size_t nw) { start(nw); }

  // DTOR
  ~Thread_Pool() { stop(); }

  template<class T>
  auto enqueue(T task)->std::future<decltype(task())>
  {
    // create a pointer (move semantics) to a wrapped std::function so that we can use std::future
    auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));

    {
      std::unique_lock<std::mutex> lock{queue_event_mutex};
      tasks_queue.emplace([=] {
        (*wrapper)();
      });
    }

    queue_event_var.notify_one();
    return wrapper->get_future();
  }

private:
  std::vector<std::thread> my_workers;

  std::condition_variable queue_event_var;

  std::mutex queue_event_mutex;
  bool stopping = false;

  std::queue<Task> tasks_queue;

  void start(size_t nw)
  {
    size_t i;
    for (i = 0; i < nw; ++i)
    {
      my_workers.emplace_back([=] {
        while (true)
        {
          Task task;
          {
            std::unique_lock<std::mutex> lock{queue_event_mutex};

            queue_event_var.wait(lock, [=] { return stopping || !tasks_queue.empty(); });

            if (stopping && tasks_queue.empty()) break;

            task = std::move(tasks_queue.front());
            tasks_queue.pop();
          }
          task();
        }
      });
    }
  }

  void stop() noexcept
  {
    {
      std::unique_lock<std::mutex> lock{queue_event_mutex};
      stopping = true;
    }

    queue_event_var.notify_all(); // every thread will start again

    for (auto &thread : my_workers)
      thread.join();
  }
};


#endif // POOL_H
