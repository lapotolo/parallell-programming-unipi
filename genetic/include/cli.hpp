#ifndef CLI_H
#define CLI_H

#include <charconv>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <system_error>

inline bool parse_size_argument(const char* text, std::size_t& value)
{
  const std::string_view input{text};
  if(input.empty()) return false;

  const auto result = std::from_chars(input.data(), input.data() + input.size(), value);
  return result.ec == std::errc{} && result.ptr == input.data() + input.size();
}

inline bool validate_common_configuration(std::size_t population_size,
                                          std::size_t chromosome_size)
{
  if(population_size == 0)
  {
    std::cerr << "population_size must be greater than zero.\n";
    return false;
  }
  if(chromosome_size < 4)
  {
    std::cerr << "chromosome_size must be at least four for the current crossover operator.\n";
    return false;
  }
  return true;
}

inline bool validate_parallel_configuration(std::size_t workers,
                                            std::size_t population_size,
                                            std::size_t chromosome_size)
{
  if(workers == 0)
  {
    std::cerr << "number_of_workers must be greater than zero.\n";
    return false;
  }
  return validate_common_configuration(population_size, chromosome_size);
}

#endif // CLI_H
