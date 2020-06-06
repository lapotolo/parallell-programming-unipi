// TODO : create a symmetric matrix for real
#ifndef TSP_GRAPH_H
#define TSP_GRAPH_H  

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <random>

class TSP_Graph
{
public:

  TSP_Graph(size_t n) : num_nodes(n) { init_tsp_graph(); };

  // operator [] returns a const reference to the i-th row of the wrapped graph
  const std::vector<uint16_t>& operator[](std::size_t i) const { return graph_m[i]; };
  
  void print_graph()
  {
    size_t k;
    std::cout<< "PRINTING THE GRAPH:\n";
    for(k=0; k < num_nodes; ++k)
    {
      for(auto e : graph_m[k]) std::cout<<e << ", ";
      std::cout<<"\n";
    }
    std::cout<<"------------------------------\n";

  }


protected:
  std::vector<std::vector<uint16_t>> graph_m;
  size_t num_nodes;
  
  // create a completely connected graph with num_nodes nodes and i.i.d weights on edges
  void init_tsp_graph()
  {  
    size_t i, z;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_w(1, 9);

    graph_m.reserve(num_nodes);
    for(i = 0; i < num_nodes; ++i)
    {
      std::vector<uint16_t> adj_list(num_nodes);
      for(z = i+1;  z < num_nodes; ++z) adj_list[z] = distrib_w(gen);

      graph_m.push_back(adj_list);
    }
  }


};

#endif // TSP_GRAPH_H
