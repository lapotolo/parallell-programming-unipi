#include <iostream>
#include <vector>
#include <functional>
#include <experimental/optional>
#include <grppi/common/patterns.h>

#include <grppi.h>


using namespace std;
using namespace std::literals::chrono_literals;

#include "utimer.hpp"

void  drain (int x) {
  this_thread::sleep_for(10ms);
  cout << x << endl;
}

int main(int argc, char * argv[]) {

  auto m = atoi(argv[1]);
  auto nw= atoi(argv[2]);
  
  grppi::dynamic_execution seq = grppi::sequential_execution{};
  grppi::dynamic_execution thr = grppi::parallel_execution_native{};
  grppi::dynamic_execution omp = grppi::parallel_execution_omp{};
  {
    utimer t("Pipeline execution"); 

    auto s1 = [] (int x) { this_thread::sleep_for(10ms); return ++x; };
    auto s2 = [] (int x) { this_thread::sleep_for(50ms); return x*x; };
    grppi::pipeline(// execution engine:
		    thr,
		    // stream generator stage
		    [m]() -> experimental::optional<int> { 
		      static int x = 0;
		      this_thread::sleep_for(10ms);
		      if (x<m) return x++;
		    else return {}; 
		    },
		    s1,
		    grppi::farm(nw,s2),
		    // stream collapser stage
		    drain
		    // end of the pipeline parameters
		    );
  }
  
  return(0);
}
