#ifndef FF_FARM_TSP_H
#define FF_FARM_TSP_H


#include <ff/utils.hpp>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include "partition.hpp"
#include "mutation.hpp"

// #include "conf.hpp"

/*
This module implements a Master-Workers ff_Farm to solve genetic TSP.
In particular the Master as an Emitter has the duty of splitting jobs
between avaiable workers while as a Collector has the duty to merge the results
returned by the workers.
*/


struct Gen_TSP_FF_Data_ptrs
{
  std::shared_ptr<std::vector<std::vector<int>>> pop;
  std::shared_ptr<std::vector<Fitness>> fit_values;
  std::shared_ptr<std::function<Fitness(std::vector<int> const&)>> fit_fun;
  std::shared_ptr<std::pair<Fitness, std::vector<int>>> curr_opt;
};


// it may represent both a range when emitted by the master and and a pair of index
// (when collected by the same master)
// where fst_idx is the position in population
// of the optimum of a single chunk while snd_idx is the position of the current worst
struct TSP_Task
{
  size_t fst_idx; // chromosome start | best
  size_t snd_idx; // exclusive chromosome end | worst
  size_t pair_fst_idx;
  size_t pair_snd_idx; // exclusive pair end
  Gen_TSP_FF_Data_ptrs ptrs; // we need to pass around pointers to data to be elaborated by farm's nodes
};


struct TSP_Master : ff::ff_monode_t<TSP_Task >
{
  // FIELDS
  size_t num_workers;
  size_t max_epochs;
  size_t population_size;
  
  size_t curr_epoch;
  size_t dispatched_curr_gen; // counter for tasks already sent in the current generation
  size_t received_curr_gen;   // counter for tasks completed in the current generation

  std::vector<TSP_Task> workers_results_to_merge;

  Gen_TSP_FF_Data_ptrs master_ptrs; // helper structs containing pointers to data structures of the problem

  // CTOR
  TSP_Master( size_t nw
            , size_t max_its
            , size_t pop_s
            , std::shared_ptr<std::vector<std::vector<int>>> pop
            , std::shared_ptr<std::vector<Fitness>> fit_values
            , std::shared_ptr<std::function<Fitness(std::vector<int> const&)>> fit_fun
            , std::shared_ptr<std::pair<Fitness, std::vector<int>>> curr_opt
            )
            : num_workers(nw)
            , max_epochs(max_its)
            , population_size(pop_s)
            , master_ptrs({pop, fit_values, fit_fun, curr_opt})
            , curr_epoch(0)
            , dispatched_curr_gen(0)
            , received_curr_gen(0)
  {}

  // split jobs and send them to workers
  void dispatch_tasks();

  // merge the results sent back by workers
  void selection(std::vector<TSP_Task> & workers_results);

  // business logic code
  TSP_Task* svc(TSP_Task* tsp_task);

};

struct TSP_Worker : ff::ff_node_t< TSP_Task, TSP_Task > 
{

  TSP_Task* svc(TSP_Task* tsp_task);

  void crossover(TSP_Task & task);
  void mutate(TSP_Task & task);
  void evaluate_population(TSP_Task & task);

};


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// FARM MASTER METHODS IMPLEMENTATION
void TSP_Master::dispatch_tasks()
{
  const auto ranges = partition_crossover_aligned(population_size, num_workers);
  for(const auto& range : ranges)
  {
    auto to_send = new TSP_Task{range.chromosomes.first,
                                range.chromosomes.second,
                                range.pairs.first,
                                range.pairs.second,
                                master_ptrs};
    ff_send_out(to_send);
    dispatched_curr_gen++;
  }
}

void TSP_Master::selection(std::vector<TSP_Task> & workers_results)
{
  auto pointer_pack = workers_results[0].ptrs;

  auto curr_gen_min_idx = workers_results[0].fst_idx;
  auto curr_gen_max_idx = workers_results[0].snd_idx;
  auto curr_gen_min_val = (*pointer_pack.fit_values)[curr_gen_min_idx];
  auto curr_gen_max_val = (*pointer_pack.fit_values)[curr_gen_max_idx];

  for(const auto& task : workers_results)
  {
    if((*pointer_pack.fit_values)[task.fst_idx] < curr_gen_min_val)
    {
      curr_gen_min_idx = task.fst_idx;
      curr_gen_min_val = (*pointer_pack.fit_values)[task.fst_idx];
    }
    if((*pointer_pack.fit_values)[task.snd_idx] > curr_gen_max_val)
    {
      curr_gen_max_idx = task.snd_idx;
      curr_gen_max_val = (*pointer_pack.fit_values)[task.snd_idx];
    }
  }

  // if in this generation we found a new optimum
  // then we record it in the proper a class field
  if(curr_gen_min_val < pointer_pack.curr_opt->first)
  {
    pointer_pack.curr_opt->first  = curr_gen_min_val;
    pointer_pack.curr_opt->second = (*pointer_pack.pop)[curr_gen_min_idx];
  }
  // inject the global optimum from previous generations in the current generation
  // in place of the worst chromosome of the current generation    
  (*pointer_pack.fit_values)[curr_gen_max_idx] = pointer_pack.curr_opt->first;
  (*pointer_pack.pop)[curr_gen_max_idx]        = pointer_pack.curr_opt->second;
}

// TSP_Master
TSP_Task* TSP_Master::svc(TSP_Task* tsp_task)
{
  if(tsp_task == nullptr) // && dispatched_curr_gen == 0)
  {
    dispatch_tasks();
    return GO_ON;
  }
  // store each workers' result in a vector on which we will perform selection
  else if(tsp_task != nullptr)
  {
    workers_results_to_merge.push_back(*tsp_task);
    delete tsp_task;
  }
  if(workers_results_to_merge.size() == dispatched_curr_gen) // if every worker sent back its result for the current gen
  {
    selection(workers_results_to_merge); // merge subresult received from workers
    dispatched_curr_gen = 0;
    workers_results_to_merge.clear();
    if( ++curr_epoch == max_epochs) return EOS;
    dispatch_tasks();
  }
  return GO_ON; // go next epoch. Right?
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// FARM WORKERS METHODS IMPLEMENTATION

// OK
void TSP_Worker::crossover(TSP_Task & task)
{
  auto pointer_pack = task.ptrs;

  size_t i, j, left, right;
  size_t chromosome_size = (pointer_pack.pop->at(0)).size();

  std::random_device rd;  // get a seed for the random number engine
  std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

  std::discrete_distribution<> biased_coin({ 1-CROSSOVER_PROB, CROSSOVER_PROB });
  
  for(size_t pair_idx = task.pair_fst_idx; pair_idx < task.pair_snd_idx; ++pair_idx)
  {
    i = 2 * pair_idx;
    if(biased_coin(gen))
    {
      std::uniform_int_distribution<> left_distr(1, ((chromosome_size)/2)-1);
      std::uniform_int_distribution<> right_distr(chromosome_size/2, chromosome_size-2);
      left  = left_distr(gen);
      right = right_distr(gen);

      // setup the structures to build in the end two feasible offspings
      std::deque<int> tmp_chromo, missing;
      std::vector<int> counter_1(chromosome_size, 0), counter_2(chromosome_size, 0);

      for(j = left; j <= right; ++j) tmp_chromo.push_back((*pointer_pack.pop)[i][j]);
      // copy central part of second parent into the central part of the first parent
      for(j = left; j <= right; ++j) (*pointer_pack.pop)[i][j] = (*pointer_pack.pop)[i+1][j];
      // viceversa, copy central part of first parent into the central part of the second parent
      for(j = left; j <= right; ++j) { (*pointer_pack.pop)[i+1][j] = tmp_chromo.front(); tmp_chromo.pop_front(); }

      // SANITIZE PHASE
      // count number of occurrences for each symbol in both the two new offsprings
      for(j = 0; j < chromosome_size; ++j) { counter_1[(*pointer_pack.pop)[i][j]]++; counter_2[(*pointer_pack.pop)[i+1][j]]++; }

      // use a deque to keep track of missing numbers of the first offspring on the front
      // and missing numbers of the second offspring in the back
      for(j = 0; j < chromosome_size; ++j) { if(counter_1[j] == 0 ) missing.push_front(j); if(counter_2[j] == 0 ) missing.push_back(j); }
      if(missing.size())
      {
        // replace doubles entries with the ones in missing
        for(j = 0; j < chromosome_size; ++j)
        {
          if(counter_1[(*pointer_pack.pop)[i][j]] == 2)
          {
            counter_1[(*pointer_pack.pop)[i][j]]--;
            counter_1[missing.front()]++;
            (*pointer_pack.pop)[i][j] = missing.front();
            missing.pop_front();
          }
          if(counter_2[(*pointer_pack.pop)[i+1][j]] == 2)
          {
            counter_2[(*pointer_pack.pop)[i+1][j]]--;
            counter_2[missing.back()]++;
            (*pointer_pack.pop)[i+1][j] = missing.back();
            missing.pop_back();
          }
        }
      } // end if(missing.size())
    } // end if(biased_cpid)
  } //end for(chunk...)
}

// OK
void TSP_Worker::mutate(TSP_Task & task)
{
  auto pointer_pack = task.ptrs;
  size_t i;
  size_t chromosome_size = (pointer_pack.pop->at(0)).size();

  std::random_device rd;  // get a seed for the random number engine
  std::mt19937 gen(rd()); // standard mersenne_twister_engine seeded with rd()

  std::discrete_distribution<> biased_coin({ 1-MUTATION_PROB, MUTATION_PROB });

  for(i=task.fst_idx; i < task.snd_idx; ++i)
    if(biased_coin(gen))
    {
      const auto positions = draw_distinct_indices(chromosome_size, gen);
      std::swap((*pointer_pack.pop)[i][positions.first],
                (*pointer_pack.pop)[i][positions.second]);
    }
}

// OK
void TSP_Worker::evaluate_population(TSP_Task & task)
{
  size_t i;
  auto pointer_pack = task.ptrs;
  auto sub_pop_min_idx = task.fst_idx;
  auto sub_pop_max_idx = task.fst_idx;

  auto sub_pop_min_val = (*pointer_pack.fit_fun)((*pointer_pack.pop)[sub_pop_min_idx]);
  auto sub_pop_max_val = sub_pop_min_val;

  for(i=task.fst_idx; i < task.snd_idx; ++i)
  { 
    (*pointer_pack.fit_values)[i] = (*pointer_pack.fit_fun)((*pointer_pack.pop)[i]);
    // looking for new best individual
    if((*pointer_pack.fit_values)[i] < sub_pop_min_val)
    {
      sub_pop_min_val = (*pointer_pack.fit_values)[i];
      sub_pop_min_idx = i;
    }
    // looking for new worst individual
    if((*pointer_pack.fit_values)[i] > sub_pop_max_val)
    {
      sub_pop_max_val = (*pointer_pack.fit_values)[i];
      sub_pop_max_idx = i;
    }
  }
  task.fst_idx = sub_pop_min_idx;
  task.snd_idx = sub_pop_max_idx;
}

// OK
TSP_Task* TSP_Worker::svc(TSP_Task* tsp_task)
{
  crossover(*tsp_task);
  mutate(*tsp_task);
  evaluate_population(*tsp_task);
  return tsp_task;
}

#endif // FF_FARM_TSP_H
