#ifndef FF_FARM_TSP_H
#define FF_FARM_TSP_H


#include <ff/utils.hpp>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include "genetic_operations.hpp"
#include "genetic_state.hpp"
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
  std::shared_ptr<GeneticState> state;
  std::shared_ptr<FitnessFunction> fitness_function;
  GeneticConfig config;
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
            , std::shared_ptr<GeneticState> state
            , std::shared_ptr<FitnessFunction> fitness_function
            , GeneticConfig config
            )
            : num_workers(nw)
            , max_epochs(max_its)
            , population_size(pop_s)
            , master_ptrs({std::move(state), std::move(fitness_function), std::move(config)})
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

struct TSP_Worker : ff::ff_node_t<TSP_Task, TSP_Task>
{
  TSP_Worker()
    : random_engine{make_random_engine()}
  {
  }

  TSP_Task* svc(TSP_Task* tsp_task);

private:
  RandomEngine random_engine;
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

void TSP_Master::selection(std::vector<TSP_Task>&)
{
  update_best_and_apply_elitism(*master_ptrs.state);
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

// FARM WORKER IMPLEMENTATION

TSP_Task* TSP_Worker::svc(TSP_Task* task)
{
  crossover_pair_range(*task->ptrs.state,
                       task->ptrs.config,
                       {task->pair_fst_idx, task->pair_snd_idx},
                       random_engine);
  mutate_range(*task->ptrs.state,
               task->ptrs.config,
               {task->fst_idx, task->snd_idx},
               random_engine);
  evaluate_range(*task->ptrs.state,
                 *task->ptrs.fitness_function,
                 {task->fst_idx, task->snd_idx});

  const auto extrema = find_fitness_extrema(
    *task->ptrs.state,
    {task->fst_idx, task->snd_idx});
  task->fst_idx = extrema.best_index;
  task->snd_idx = extrema.worst_index;
  return task;
}

#endif // FF_FARM_TSP_H
