#include "../include/genetic_tsp_par.hpp"
#include "../include/tsp_graph.hpp"
#include "../include/validation.hpp"
#include "../include/cli.hpp"
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>


int main(int argc, char const *argv[])
{
	if(argc != 5 && argc != 6) // nw, niter, pop_size, chromo_size, cross_prob, mutate_prob
  {
		std::cout << "Parallel (naive forks/joins version) Genetic TSP Usage is: <number_of_workers> <max_epochs> <population_size> <chromosome_size> [seed]\nShutting down.\n";
		return -1;
	}

  size_t nw = 0;
  size_t max_epochs = 0;
  size_t pop_size = 0;
  size_t chromo_size = 0;
  if(!parse_size_argument(argv[1], nw) ||
     !parse_size_argument(argv[2], max_epochs) ||
     !parse_size_argument(argv[3], pop_size) ||
     !parse_size_argument(argv[4], chromo_size) ||
     !validate_parallel_configuration(nw, pop_size, chromo_size))
    return -1;
  RandomSeed seed = make_random_seed();
  if(argc == 6 && !parse_seed_argument(argv[5], seed))
  {
    std::cerr << "invalid seed.\n";
    return -1;
  }

  // create a complete weighted graph with #chromo_size numbers on node
  // edges' weights are i.i.d from the range [1,100]
  TSP_Graph test_graph(chromo_size, seed);

  // tried to overload operator() but strangely didnt work :()
  auto fit_funct = [&](const Tour& chromo)
                      {
                        Fitness tour_cost = 0;
                        std::size_t k;
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

  const GeneticConfig config{pop_size, chromo_size, max_epochs};

  Genetic_TSP_Parallel test(nw, config, fit_funct, seed);

  std::cerr << "seed=" << seed << '\n';

  // Parallel EXECUTION
  auto start = std::chrono::high_resolution_clock::now();

  test.run();

  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  auto usec    = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

  const auto best_solution = test.get_current_optimum();
  if(!validate_best_solution(best_solution, chromo_size, fit_funct)) return -1;



  // WRITE RESULTS ON A FILE FOR FUTURE ANALYSIS
  std::ofstream out_file;
  out_file.open( "results/runs/"
               + (std::to_string(max_epochs))
               + "-max_epochs-"
               + (std::to_string(pop_size))
               + "-chromo-"
               + (std::to_string(chromo_size))
               + "-cities-"
               + (std::to_string(nw))
               +"-nw_par.data"
               , std::ios::app);
  out_file << usec << "\n";
  out_file.close();

  // RESULTS PRINTINGS
  //std::cout<<"*****\nopt      = " << test.get_current_optimum().first << "\n";
  //std::cout<<"glob opt tour= [ ";
  //for(auto e : test.get_current_optimum().second) std::cout<< e << " ";
  //std::cout<<"]\n";
  std::cout << "t_par("<<nw<<")=" << usec << "\n";

  return 0;
}
