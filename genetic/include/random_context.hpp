#ifndef RANDOM_CONTEXT_H
#define RANDOM_CONTEXT_H

#include "domain.hpp"
#include "mutation.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

using RandomSeed = std::uint64_t;
using RandomEngine = std::mt19937_64;

struct CrossoverDecision
{
  bool apply = false;
  std::size_t left = 0;
  std::size_t right = 0;
};

inline bool operator==(const CrossoverDecision& first,
                       const CrossoverDecision& second)
{
  return first.apply == second.apply &&
         first.left == second.left &&
         first.right == second.right;
}

struct MutationDecision
{
  bool apply = false;
  std::size_t first = 0;
  std::size_t second = 0;
};

inline bool operator==(const MutationDecision& first,
                       const MutationDecision& second)
{
  return first.apply == second.apply &&
         first.first == second.first &&
         first.second == second.second;
}

struct GenerationRandomPlan
{
  std::vector<CrossoverDecision> crossover;
  std::vector<MutationDecision> mutation;
};

inline bool operator==(const GenerationRandomPlan& first,
                       const GenerationRandomPlan& second)
{
  return first.crossover == second.crossover &&
         first.mutation == second.mutation;
}

inline RandomSeed make_random_seed()
{
  std::random_device device;
  std::seed_seq sequence{
    device(), device(), device(), device(),
    device(), device(), device(), device()
  };
  std::vector<std::uint32_t> words(2);
  sequence.generate(words.begin(), words.end());
  return (static_cast<RandomSeed>(words[0]) << 32U) |
         static_cast<RandomSeed>(words[1]);
}

inline RandomEngine make_random_engine(RandomSeed seed)
{
  return RandomEngine{seed};
}

inline RandomSeed derive_seed(RandomSeed seed, RandomSeed stream) noexcept
{
  auto value = seed + 0x9E3779B97F4A7C15ULL * (stream + 1ULL);
  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
  return value ^ (value >> 31U);
}

class RandomContext
{
public:
  explicit RandomContext(RandomSeed seed)
    : seed_{seed}
    , engine_{seed}
  {
  }

  [[nodiscard]] RandomSeed seed() const noexcept
  {
    return seed_;
  }

  RandomEngine& engine() noexcept
  {
    return engine_;
  }

  GenerationRandomPlan make_generation_plan(const GeneticConfig& config)
  {
    GenerationRandomPlan plan;
    plan.crossover.resize(config.population_size / 2);
    plan.mutation.resize(config.population_size);

    std::bernoulli_distribution should_crossover{
      config.crossover_probability};
    std::uniform_int_distribution<std::size_t> left_distribution{
      1, config.chromosome_size / 2 - 1};
    std::uniform_int_distribution<std::size_t> right_distribution{
      config.chromosome_size / 2, config.chromosome_size - 2};

    for(auto& decision : plan.crossover)
    {
      decision.apply = should_crossover(engine_);
      if(!decision.apply) continue;
      decision.left = left_distribution(engine_);
      decision.right = right_distribution(engine_);
    }

    std::bernoulli_distribution should_mutate{
      config.mutation_probability};
    for(auto& decision : plan.mutation)
    {
      decision.apply = should_mutate(engine_);
      if(!decision.apply) continue;

      const auto positions = draw_distinct_indices(
        config.chromosome_size,
        engine_);
      decision.first = positions.first;
      decision.second = positions.second;
    }

    return plan;
  }

private:
  RandomSeed seed_;
  RandomEngine engine_;
};

#endif // RANDOM_CONTEXT_H
