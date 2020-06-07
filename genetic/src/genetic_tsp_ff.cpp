#include "../include/genetic_tsp_ff.hpp"
#include "../include/tsp_graph.hpp"

int main(int argc, char const *argv[])
{
	if(argc != 1+6) // nw, niter, pop_size, chromo_size, cross_prob, mutate_prob
  {
		std::cout << "Parallel Genetic TSP with FastFlow Usage is: <number_of_workers> <max_epochs> <population_size> <chromosome_size> <cross_probability> <mutation_probability>\nShutting down.\n";
		return -1;
	}

  size_t nw          = atoi(argv[1]);
  size_t max_epochs  = atoi(argv[2]);
  size_t pop_size    = atoi(argv[3]);
  size_t chromo_size = atoi(argv[4]);
  float cross_prob  = atof(argv[5]);
  float mutat_prob  = atof(argv[6]);

  size_t epoch = 0;

  // create a complete weighted graph with #chromo_size numbers on node
  // edges' weights are i.i.d from the range [1,100]
  TSP_Graph test_graph(chromo_size);

  // test_graph.print_graph();

  // tried to overload operator() but strangely didnt work :()
  auto fit_funct = [&](std::vector<int> const& chromo)
                      {
                        uint32_t tour_cost = 0;
                        size_t k, i, j; 
                        for(k = 0; k < chromo_size-1; ++k)
                        {
                          // since the graph yields a symmetric matrix permute indexes so that only the upper triangular part is accessible
                          if( chromo[k] < chromo[k+1]) { tour_cost += test_graph[chromo[k]][chromo[k+1]]; }
                          else { tour_cost += test_graph[chromo[k+1]][chromo[k]]; }
                        }
                        if( chromo[0] < chromo[chromo_size-1]) { tour_cost += test_graph[chromo[0]][chromo[chromo_size-1]]; }
                        else { tour_cost += test_graph[chromo[chromo_size-1]][chromo[0]]; }
                        return tour_cost;
                      };
  
  Genetic_TSP_FF test( nw
                     , pop_size 
                     , chromo_size
                     , cross_prob
                     , mutat_prob
                     , fit_funct
                     );

  { // Parallel FastFlow EXECUTION
    auto start = std::chrono::high_resolution_clock::now();

                          
    while(epoch < max_epochs) 
    { 
      // std::cout<<"epoch: "<<epoch<< " | curr min:" << test.get_current_optimum().first << "\n";
      test.next_generation(nw);
      ++epoch;
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto usec    = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout<<"\nParallel (FastFlow) execution on a completely connected random graph with " << chromo_size << " nodes took (usec): "<< usec << std::endl;
  
    //std::cout << "Speedup is " << ((float) tseq)/((float) msec) << " (should be " << nw << ")" << std::endl; 

  }

  auto result = test.get_current_optimum().first;
  auto v      = test.get_current_optimum().second;

  std::cout<<"min cost ham tour: " << result <<"\n";
    
  std::cout<<"opt tour:\n[";
  for( auto e : v) std::cout<<e << ",";
  std::cout<<"]\n";
  
  // no duplicates tests
  // std::sort(v.begin(), v.end());
  // for( auto e : v) std::cout<<e << ",";
  // std::cout<<"\n";

  return 0;
}
