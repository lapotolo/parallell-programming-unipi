#include <ff/utils.hpp>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>


#include <thread>

// it may represent both a range when emitted by the master and and a pair of index
// (when collected by the same master)
// where fst_idx is the position in population
// of the optimum of a single chunk while snd_idx is the position of the current worst
struct TSP_Task
{
  size_t fst_idx;
  size_t snd_idx;
};

/*

fai una farm master-worker ie: emitter fa sia da collector sia da emitter

crea una struct TSP_Worker il cui svc è  crossover->mutate->eval


*/
struct TSP_Master : ff::ff_node_t<TSP_Task >
{
  TSP_Master(size_t max_its) : max_epochs(max_its), curr_epoch(0) {}

  TSP_Task* svc(TSP_Task* tsp_task)
  {
    if(tsp_task == nullptr) // first run of ranges
    {
      // create_chunks();
      //for every range to give away to workers
      //{
      //  ff_send_out(range)
      //}
      return GO_ON;
    } // OTHERWISE WE ARE RECEIVING FROM THE FEEDBACK CHANNEL COMING FROM THE WORKERS
    // deal with selection by merging optima received in tsp task
    // [...]
    // BARRIER TO WAIT FOR ALL THE SEGMENTS BACK FROM WORKERS FOR THIS GENERATION
    
    // WHEN WE GOT ALL THE OPTS OF THIS GENERATION DO THE SELECTION
    curr_epoch++;
    if( curr_epoch >= max_epochs) return EOS;
    return GO_ON;
  }

  size_t curr_epoch;
  size_t max_epochs;
  std::vector<TSP_Task> tasks_to_sendout;
};


struct TSP_Worker : ff::ff_node_t< TSP_Task, TSP_Task> 
{
  // receive a range. 
  // on that range apply crossover, mutation and perform evaluatio.
  // ff_send_out(a pair)
};

void Genetic_TSP_FF::run()
{

  size_t i;
  TSP_Master master(max_epochs);

  // create the vector keeping pointers for farm's workers
  std::vector<std::unique_ptr<ff::ff_node>> tsp_workers;
  for(i = 0; i < num_workers; ++i)
    tsp_workers.push_back(ff::make_unique<TSP_Worker>());

  // create the farm and set its topology (Master-Worker)
  ff::ff_Farm<TSP_Task> farm_gene_tsp(std::move(tsp_workers), master);
  farm_gene_tsp.remove_collector();
  farm_gene_tsp.wrap_around();

  ff::ffTime(ff::START_TIME);
  if (farm_gene_tsp.run_and_wait_end()<0)
  {
    ff::error("running farm");
    return;
  }
  ff::ffTime(ff::STOP_TIME);
  std::cout << "Time: " << ff::ffTime(ff::GET_TIME) << "\n";
  return;


}


