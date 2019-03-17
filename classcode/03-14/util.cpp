#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <string>

//
// needed a blocking queue
// here is a sample queue
//

// #define DEBUG

template <typename T>
class aqueue
{
private:
  std::mutex              d_mutex;
  std::condition_variable d_condition;
  std::deque<T>           d_queue;
  std::string             d_s; 
public:

  aqueue(std::string s) {
    d_s = s; 
#ifdef DEBUG
    std::cout << "Created " << s << " queue " << std::endl;
#endif
    return;
  }
  aqueue() {}
  
  void push(T const& value) {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
#ifdef DEBUG
      std::cout << "Pushing " << value << " on queue " << d_s << std::endl;
#endif
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }
  
  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
    T rc(std::move(this->d_queue.back()));
    this->d_queue.pop_back();
#ifdef DEBUG
    std::cout << "Popping " << rc << " from queue " << d_s << std::endl;
#endif
    return rc;
  }
};


//
// needed something to represent the EOS
// here we use null
//
#define EOS NULL


// loose some time
void active_delay(int usecs) {
  // read current time
  auto start = std::chrono::high_resolution_clock::now();
  auto end   = false;
  while(!end) {
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if(usec>usecs)
      end = true;
  }
  return;
}

// double items in a vector, adding some delay
void double_vec_item(std::vector<double>& v, int msec) {
  for(int i=0; i<v.size(); i++) {
    active_delay(msec);
    v[i] = v[i]*2;
  }
  return;
}

// increases items in vector, adding some delay
void inc_vec_item(std::vector<double>& v,int msec) {
  for(int i=0; i<v.size(); i++) {
    active_delay(msec);
    v[i] = v[i]+1;
  }
  return;
}

// print items in a vector, just for check
void print_vec(std::vector<double> v) {
  for(auto it : v)
    std::cout << it << " ";
  std::cout << std::endl;
  return;
}
