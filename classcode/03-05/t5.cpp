#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#include <unistd.h>

//using namespace std;

std::mutex l; 
int x = 0;                  // this is global

void body(int n) {
  l.lock();
  int y = x;
  y = y + n;
  sleep(0.3); 
  x = y;
  // x += n; 
  l.unlock();
  return;
}

int main(int argc, char * argv[]) {

  int n = atoi(argv[1]);    // segv if not present ...

  std::vector<std::thread> threads_vector;

  for(size_t i = 0; i < n; ++i) threads_vector.push_back(std::thread(body,i));

  for(std::thread& it: threads_vector) it.join();

  std::cout << "x = " << x << std::endl; 
  return(0); 
}
