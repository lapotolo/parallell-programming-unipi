#ifndef TSP_GRAPH_H
#define TSP_GRAPH_H

#include "domain.hpp"
#include "random_context.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <random>
#include <stdexcept>
#include <vector>

class TspGraph
{
public:
  using Weight = std::uint16_t;

  explicit TspGraph(std::size_t node_count,
                    RandomSeed seed = make_random_seed())
    : weights_(node_count, std::vector<Weight>(node_count))
  {
    initialize(seed);
  }

  [[nodiscard]] std::size_t node_count() const noexcept
  {
    return weights_.size();
  }

  [[nodiscard]] Weight weight(City first, City second) const
  {
    if(first >= node_count() || second >= node_count())
      throw std::out_of_range{"city index is outside the graph"};
    if(first == second) return 0;
    return first < second
         ? weights_[first][second]
         : weights_[second][first];
  }

  [[nodiscard]] Fitness tour_cost(const Tour& tour) const
  {
    if(tour.size() != node_count())
      throw std::invalid_argument{"tour size does not match graph size"};

    Fitness cost = 0;
    for(std::size_t index = 0; index < tour.size(); ++index)
    {
      const auto next = (index + 1) % tour.size();
      cost += weight(tour[index], tour[next]);
    }
    return cost;
  }

  void print(std::ostream& output = std::cout) const
  {
    output << "PRINTING THE GRAPH:\n";
    for(const auto& row : weights_)
    {
      for(const auto value : row)
        output << value << ", ";
      output << '\n';
    }
    output << "------------------------------\n";
  }

private:
  std::vector<std::vector<Weight>> weights_;

  void initialize(RandomSeed seed)
  {
    RandomEngine engine{seed};
    std::uniform_int_distribution<unsigned int> weight_distribution{1, 9};

    for(std::size_t first = 0; first < node_count(); ++first)
    {
      for(std::size_t second = first + 1; second < node_count(); ++second)
      {
        weights_[first][second] = static_cast<Weight>(
          weight_distribution(engine));
      }
    }
  }
};

#endif // TSP_GRAPH_H
