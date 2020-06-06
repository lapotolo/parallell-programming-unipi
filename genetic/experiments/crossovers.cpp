// BUG: when

#include <vector>
#include <utility>
#include <iostream>
#include <functional>
#include <random>
#include <algorithm>


std::vector<int> ParGenAlgTSP::PMX(std::vector<int> p1, std::vector<int> p2, int cross_point) {
    std::vector<int> offspring1(p1);
    int index;
    std::vector<int>::iterator it;
    for (int j = 0; j < cross_point; j++) {
        it = std::find(offspring1.begin(), offspring1.end(), p2[j]);
        index = std::distance(offspring1.begin(), it);
        std::swap(offspring1[j], offspring1[index]);
    }
    return offspring1;
}





void sanitize(std::vector<int> & bad_chromosome) 
{
  return; // TODO
}
void crossover(uint32_t const& chunk_s, uint32_t const& chunk_e)
{
  uint32_t i, j, left, right;
  // *********************************************************************************
  // *********************************************************************************
  // DEB
  size_t i;
  size_t population_size = 100; // rows
  size_t chromosome_size = 100; // columns
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, chromosome_size-1);


  std::vector<std::vector<int>> population;
  population.reserve(population_size);
  
  for(i = 0; i < population_size; ++i)
  {
    std::vector<int> chromosome(chromosome_size);
    std::iota(chromosome.begin(), chromosome.end(), 0);
    std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});
    population.push_back(chromosome);
  }



  float crossover_prob   = 0.3;

  // END DEB
  // *********************************************************************************
  // *********************************************************************************

  std::vector<int> tmp_chromo_1(population_size);
  std::vector<int> tmp_chromo_2(population_size);

  std::random_device rd;  // get a seed for the random number engine
  std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()
  
  std::discrete_distribution<> biased_coin({ 1-crossover_prob, crossover_prob });

  std::uniform_int_distribution<> left_idx_distr(1, (chromosome_size-1)>>1); // generate a random index between [1, chromosome.size/2] ie. cannot generate the first index
  std::uniform_int_distribution<> right_idx_distr(((chromosome_size-1)>>1) + 1, chromosome_size-2); // generate a random index between [chromosome.size/2, chromosome.size-2] ie. cannot generate the last index
  
  for(i=0; i < population_size; i+=2)
  {
    if(biased_coin(gen))
    {
      tmp_chromo_1 = population[i];
      tmp_chromo_2 = population[i+1];
      left  = left_idx_distr(gen);
      right = right_idx_distr(gen);
      
      // copy central part of second parent into the central part of the first parent
      for(j = left; j <= right; ++j) population[i][j] = tmp_chromo_2[j];

      // viceversa, copy central part of first parent into the central part of the second parent
      for(j = left; j <= right; ++j) population[i+1][j] = tmp_chromo_1[j];

      // DEBUG
      std::cout<<"\nCROSSOVER DONE\nGenerated chromosomes are: \n";
      for(j = 0; j < chromosome_size; ++j) std::cout<< population[i][j] <<",";
      std::cout<<"\n";
      for(j = 0; j < chromosome_size; ++j) std::cout<< population[i+1][j] <<",";
      std::cout<<"\n***************\n";
      
      tmp_chromo_1 = population[i];
      tmp_chromo_2 = population[i+1];

      std::sort(tmp_chromo_1.begin(), tmp_chromo_1.end());
      std::sort(tmp_chromo_2.begin(), tmp_chromo_2.end());
      for(j = 0; j < chromosome_size; ++j) std::cout<< tmp_chromo_1[j] <<",";
      std::cout<<"\n";
      for(j = 0; j < chromosome_size; ++j) std::cout<< tmp_chromo_2[j] <<",";
      std::cout<<"\n";
      
      // END DEBUG
      sanitize(population[i]);
      sanitize(population[i+1]);
    }
  }
}

int main()
{

}