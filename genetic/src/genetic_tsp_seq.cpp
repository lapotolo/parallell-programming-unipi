#include "../include/genetic_tsp_seq.hpp"
#include "../include/tsp_graph.hpp"

int main(int argc, char const *argv[])
{
  if(argc != 1+5) // niter, pop_size, chromo_size, cross_prob, mutate_prob
  {
    std::cout << "Sequential Genetic TSP Usage is: <max_epochs> <population_size> <chromosome_size> <cross_probability> <mutation_probability>\nShutting down.\n";
    return -1;
  }

  size_t max_epochs  = atoi(argv[1]);
  size_t pop_size    = atoi(argv[2]);
  size_t chromo_size = atoi(argv[3]);
  float  cross_prob  = atof(argv[4]);
  float  mutat_prob  = atof(argv[5]);

  size_t epoch = 0;

  // create a complete weighted graph with #chromo_size numbers on node
  // edges' weights are i.i.d from the range [1,100]
  TSP_Graph test_graph(chromo_size);

  // test_graph.print_graph();

  // tried to overload operator() but strangely didnt work :(
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
  
  // get an instance of the mini framework representing genetic algorithms
  Genetic_TSP_Sequential test( max_epochs
                             , pop_size 
                             , chromo_size
                             , cross_prob
                             , mutat_prob
                             , fit_funct
                             );

  // SEQUENTIAL EXECUTION
  auto start = std::chrono::high_resolution_clock::now();

  test.run();

  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  auto usec    = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

  auto result = test.get_current_optimum().first;

  std::ofstream out_file;
  out_file.open( "results/"
               + (std::to_string(max_epochs))
               + "-max_epochs-"
               + (std::to_string(pop_size))
               + "-chromo-"
               + (std::to_string(chromo_size))
               + "-cities"
               + "_seq.data"
               , std::ios::app);
  out_file << usec << "\n";
  out_file.close();

  // DEBUG PRINTINGS
  std::cout<<"\n***\nglob opt = " << test.get_current_optimum().first << "\n";
  std::cout<<"glob opt tour= [ ";
  for(auto e : test.get_current_optimum().second) std::cout<< e << " ";
  std::cout<<"]\n";
  std::cout << "time= " << usec << "\n";

  return 0;
}
