#include <iostream>
#include <omp.h>

#define DEBUG

int fibonacci(int i) {
  int res;
#ifdef DEBUG
  // #pragma omp single
  {
    std::cout << "Computing fib(" << i << ") on thread " << omp_get_thread_num() << " on thread context " << sched_getcpu() <<  std::endl;
  }
#endif
#ifdef CORES
  std::cout << sched_getcpu() << " " << std::flush; 
#endif
  
  if (i<2)
    res = 1;
  else {
    int i1 = 0;
    // missing the shared -> does not work : i1 = firstprivate (default)
#pragma omp task shared(i1)
    {
      i1 = fibonacci(i-1);
    }

    int i2 = 0;
    // #pragma omp task shared(i2)
    {
      i2 = fibonacci(i-2);
    }

#pragma omp taskwait

    res = i1 + i2;
  }
  return res;
}

int main(int argc, char * argv[]) {

  int n = atoi(argv[1]);
  int nw = atoi(argv[2]);
  int res = 0;
  
#pragma omp parallel num_threads(nw)
  {
    res = fibonacci(n);
  }
  std::cout << "Eventually: fibonacci(" << n << ")=" << res << std::endl;

  return(0);
}
