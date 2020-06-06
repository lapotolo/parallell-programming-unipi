#include <vector>
#include <iostream>
#include <functional>
#include <random>
#include <algorithm>
#include <thread>

int main()
{
  auto f = [](int a, int b){ auto s = a+b; return s;};
  size_t nw = 10;
  std::vector<std::thread> workers(nw);
  
  
  for(int i = 0; i < nw; ++i) 
  {
    workers.push_back(std::move(std::thread( f, 1, 2)));
  }

  for(auto & thr : workers) thr.join(); // JOIN
  uint32_t i, j;

  uint32_t pop_size = 10;
  uint32_t chromosome_size = 10;
  
  std::vector<std::vector<int>> population;

  population.reserve(pop_size);
  for(i = 0; i < pop_size; ++i)
  {
    std::vector<int> chromosome(chromosome_size);
    std::iota(chromosome.begin(), chromosome.end(), 0);

    std::shuffle(chromosome.begin(), chromosome.end(), std::mt19937{std::random_device{}()});

    population.push_back(chromosome);

  }
  for(i = 0; i < pop_size; ++i)
  {
    for (auto e : population[i]) std::cout<< e << " ";
    std::cout<<"\n";

  }
  float s = std::accumulate( std::begin(population[0])
                          , std::end(population[0])
                          , 0);

  std::cout<<"\n"<< 1/(s+1)<< "\n";
  
  uint32_t l;
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, chromosome_size-1);
  
  int id1, id2;
  for(l=0; l < pop_size; ++l)
  {
    id1 = distrib(gen);
    id2 = distrib(gen);
    std::cout<<"(" << id1 <<", "<<id2<<") ==> ";

    // std::swap(population[l][id1], population[l][id2]);
    
    for (auto e : population[l]) std::cout<< e << " ";
    std::cout<<"\n";
  }

  float mutation_prob = 0.3; 
  std::discrete_distribution<> distrib_coin({ 1-mutation_prob, mutation_prob });

  s = 0;
  for(l=0; l < 100000; ++l) 
  {
    if(distrib_coin(gen)) s++;
  }
  std::cout<<"ones: " << s <<"\n";
  std::cout<<"test distrib: " << (float)s/100000<<"\n";

  for(l = 0; l<100; ++l)
  {
    auto coin = distrib_coin(gen);
    std::cout<<"Mut coin: " << coin<<"\n";
  }
  std::uniform_int_distribution<> left_idx_distr(1, (chromosome_size-1)>>1);
  std::uniform_int_distribution<> right_idx_distr(((chromosome_size-1)>>1) + 1, chromosome_size-2);

  for(int k = 0; k< 50; ++k) std::cout<< left_idx_distr(gen) << ", ";
  std::cout<<"\n";
  for(int k = 0; k< 50; ++k) std::cout<< right_idx_distr(gen) << ", ";
  std::cout<<"\n";

  // *********************************************************************

  return 0;



}