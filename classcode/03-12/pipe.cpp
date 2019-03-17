#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

#include <omp.h>

std::vector<int>  x;    // tasks
std::vector<int>  y;    // f(task)
std::vector<int>  z;    // g(f(task))


int main(int argc, char * argv[]) {

  if(argc==1) {
    std::cerr << "Usage is: " << argv[0] << " m nw " << std::endl;
    return 0;
  }
  
  int m    = atoi(argv[1]);
  int nw   = atoi(argv[2]);
  int seed = atoi(argv[3]);

  srand(seed);
  
  x.reserve(m);
  y.reserve(m);
  z.reserve(m);
  
  auto f = [] (int i) { return(++i); };
  auto g = [] (int i) { return(i*i); };
  
#pragma omp parallel num_threads(nw)
  {
#pragma omp single
    {
      for(int i=0; i<m; i++) {
#pragma omp task depend(out: (x[i]))
	{
	  int s = ((int) (rand() * 2));
	  if(s!=0)
	    std::this_thread::sleep_for(1s);
	  
	  x[i] =  i;        // create a task
	}
#pragma omp task depend( in : (x[i])) depend( out : (y[i]))
	{
	  int s = ((int) (rand() * 2));
	  if(s!=0)
	    std::this_thread::sleep_for(1s);

	  y[i] =  f(x[i]);  // compute first stage
	}
#pragma omp task depend( in : (y[i])) depend( out : (z[i]))
	{
	  int s = ((int) (rand() * 2));
	  if(s!=0)
	    std::this_thread::sleep_for(1s);

	  z[i] =  g(y[i]);  // compute first stage
	  std::cout << z[i]  << " " << std::flush; 
	}
	
      }
    }
  }
  
  std::cout << "Eventually ... " << std::endl;
  for(int i=0; i<m; i++) {
    std::cout << z[i]  << " ";
  }
  std::cout << std::endl;
  return 0;
}
