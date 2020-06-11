#include <ff/utils.hpp>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>


#include <thread>

/*
This module implements a Master-Workers ff_Farm to solve genetic TSP.
In particular the Master as the Emitter has the duty of splitting jobs
between avaiable workers while as the Collector has the duty to merge the results
returned by the workers.
*/


// it may represent both a range when emitted by the master and and a pair of index
// (when collected by the same master)
// where fst_idx is the position in population
// of the optimum of a single chunk while snd_idx is the position of the current worst
struct TSP_Task
{
  size_t fst_idx; // start | best
  size_t snd_idx; // end   | worst
};

struct TSP_Master : ff::ff_node_t<TSP_Task >
{
  // FIELDS
  std::vector<TSP_Task> workers_results_to_merge;

  size_t num_workers;
  size_t max_epochs;
  size_t population_size;
  
  size_t curr_epoch;
  size_t dispatched_curr_gen;
  size_t received_curr_gen;

  TSP_Master( size_t nw
            , size_t max_its
            , size_t pop_s
            )
            : num_workers(nw)
            , max_epochs(max_its)
            , population_size(pop_s)
            , curr_epoch(0)
            , dispatched_curr_gen(0)
            , received_curr_gen(0)
  {}

  void dispatch_tasks()
  {
    size_t i, step;
    step = population_size / num_workers;
    for (i = 0; i + step - 1 < population_size; i += step) 
    {
      auto to_send = (TSP_Task{i, i+step +1});
      ff_send_out(&to_send);
      dispatched_curr_gen++;
    }
  }

  TSP_Task* svc(TSP_Task* tsp_task) // dispatched_tasks CANNOT WORK
  {
    if(tsp_task == nullptr) // first run of ranges
    {
      // spit out tasks to workers
      dispatch_tasks(); 
      return GO_ON;
    } // OTHERWISE WE ARE RECEIVING FROM THE FEEDBACK CHANNEL COMING FROM THE WORKERS
    // store each workers' result in a vector on which we will perform selection
    workers_results_to_merge.push_back(*tsp_task);
    delete tsp_task;
    // keep track of how many workers sent back a result for the current generation
    received_curr_gen++;
    // BARRIER TO WAIT FOR ALL THE SEGMENTS BACK FROM WORKERS FOR THIS GENERATION    
    if(received_curr_gen == dispatched_curr_gen) // if every worker sent back its result for the current gen
    {
      selection(workers_results_to_merge);       // WHEN WE GOT ALL THE OPTS OF THIS GENERATION DO THE SELECTION
      curr_epoch++;                              // go next gen
      dispatched_curr_gen = 0;
      received_curr_gen   = 0;
      if( curr_epoch >= max_epochs) return EOS;
    } 
    return GO_ON; // go next epoch. Right?
  }

};

struct TSP_Worker : ff::ff_node_t< TSP_Task, TSP_Task > 
{

  TSP_Task* svc(TSP_Task* tsp_task)
  {
    auto start = tsp_task->fst_idx;
    auto end   = tsp_task->snd_idx;
    crossover(start, end);
    mutate(start, end);
    ff_send_out(&(evaluate_population(start, end)));
    return GO_ON;
  }

};

void Genetic_TSP_FF::run()
{

  size_t i;
  TSP_Master master(num_workers, max_epochs, population_size);

  // create the vector keeping pointers for farm's workers
  std::vector<std::unique_ptr<ff::ff_node>> tsp_workers;
  for(i = 0; i < num_workers; ++i)
    tsp_workers.push_back(ff::make_unique<TSP_Worker>());

  // create the farm and set its topology (Master-Worker)
  ff::ff_Farm<TSP_Task> farm_gene_tsp(std::move(tsp_workers), master);
  farm_gene_tsp.remove_collector();
  farm_gene_tsp.wrap_around();

  ff::ffTime(ff::START_TIME);
  if(farm_gene_tsp.run_and_wait_end() < 0)
  {
    ff::error("running farm");
    return;
  }
  ff::ffTime(ff::STOP_TIME);
  std::cout << "Time: " << ff::ffTime(ff::GET_TIME) << "\n";
  return;
}