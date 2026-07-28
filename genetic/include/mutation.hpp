#ifndef MUTATION_H
#define MUTATION_H

#include <cstddef>
#include <random>
#include <stdexcept>
#include <utility>

template<typename RandomEngine>
std::pair<std::size_t, std::size_t> draw_distinct_indices(std::size_t size,
                                                         RandomEngine& engine)
{
  if(size < 2)
    throw std::invalid_argument("at least two positions are required for swap mutation");

  std::uniform_int_distribution<std::size_t> first_distribution(0, size - 1);
  const auto first = first_distribution(engine);

  // Draw from n-1 values and skip over `first`; this terminates in one draw.
  std::uniform_int_distribution<std::size_t> second_distribution(0, size - 2);
  auto second = second_distribution(engine);
  if(second >= first) ++second;

  return {first, second};
}

#endif // MUTATION_H
