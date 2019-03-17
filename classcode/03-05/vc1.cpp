#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <string>
#include <thread>

using namespace std::literals::chrono_literals;

//
// needed a blocking queue
// here is a sample queue
//

template <typename T>
class sample_queue
{
private:
  std::mutex              d_mutex;
  std::condition_variable d_condition;
  std::deque<T>           d_queue;
public:

  sample_queue(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }
  sample_queue() {}
  
  void push(T const& value) {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }
  
  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
    return rc;
  }

  bool is_empty() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    return(d_queue.empty());
  }
};

int main(int argc, char * argv[]) {

  sample_queue<int> Q;
  
  auto body = [&] (int k) {
    // add i-th param from command line to the queue
    int kv = atoi(argv[k]);
    std::this_thread::sleep_for(10ms);
    Q.push(kv); 
    return;
  };

  std::vector<std::thread*> tid;

  tid.resize(0);
  for(int i=1; i<argc; i++) {
    tid.push_back(new std::thread(body,i));
  }
  for(auto &t : tid) {
    t->join();
  }

  while(!Q.is_empty()) {
    std::cout << Q.pop() << " ";
  }
  std::cout << std::endl;
  
  return(0);
}
