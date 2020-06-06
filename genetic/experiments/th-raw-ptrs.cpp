
template<>
void Genetic_Algorithm< std::vector<std::vector<int>>
                      , std::vector<int>
                      , int32_t
                      >::next_generation(size_t nw)
{
  // ************************************************
  auto tstart = std::chrono::high_resolution_clock::now();

  // ************************************************
  size_t i;
  size_t chunk_size = chromosome_size / nw; // number of chromosome that each worker have to deal with

  std::vector<std::thread*> t_ids(nw);
  
  for(i=0; i < nw; ++i) t_ids[i] = new std::thread(crossover, i*chunk_size, (i+1)*chunk_size);  // FORK nw threads;
  for(i=0; i < nw; ++i) t_ids[i]->join(); // JOIN
  // ---- barrier ----
  for(i=0; i < nw; ++i) t_ids[i] = new std::thread(mutate, i*chunk_size, (i+1)*chunk_size);  // FORK nw threads;
  for(i=0; i < nw; ++i) t_ids[i]->join(); // JOIN
  // ---- barrier ----
  evaluate_population(0, population_size);
  // barrier

}
