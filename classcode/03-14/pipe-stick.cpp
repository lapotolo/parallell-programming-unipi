#include <iostream>
#include <functional>
#include <thread>
#include <chrono>
#include <string>
#include <sched.h>

#include "util.cpp"
#include "utimer.cpp"

using namespace std::literals::chrono_literals;

// define the end of stream marker for streams of positive integers (as the one assumed here)
#define INTEOS (-1)

// flags to control the information printed during the execution and the thread to core sticking
bool tracecores = false;
bool fixthreads = false;

// force a given std::thread to be executed on one of the thread contexts available
void stickCurrentThread(std::string name, std::thread *tid, int coreno) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(coreno, &cpuset);  // without % those in excess are free to move
  if(pthread_setaffinity_np(tid->native_handle(),sizeof(cpu_set_t), &cpuset) != 0) {
    auto reason = errno; 
    std::cerr << "Error while sticking current thread (" << name << " ) to core: "
	      << (reason == EINVAL ? "EINVAL" : (reason == EFAULT ? "EFAULT" : (reason == ESRCH ? "ESRCH" : "UNKNOWN"))) 
	      << std::endl;
  } else {
    std::cerr << "Thread (" << name << ") sticked to core " << coreno << std::endl;
  }
  return;
}

// this is used to increase the time spent in each one of the stages
std::chrono::microseconds delay;

// class Stage wrapper: implements a thread reading from input queue, computing a parameter function
// and delivering results to the output queue

template<typename In, typename Out> class Stage {
private:
  std::function<Out(In)> f;           // the parameter function to be computed by the stage
  aqueue<In>& inQ;                    // the input queue
  aqueue<Out>& outQ;                  // the output queue
  std::string name;                   // name of the stage ... just for debug info printing
  int coreno;                         // core to use for the thread 
  bool stickcore;                     // flag telling whether the thread has to be assigned to a core
public:
  // non sticking contructor
  Stage(std::string name, std::function<Out(In)> f, aqueue<In>& inQ, aqueue<Out>& outQ):
    name(name), f(f), inQ(inQ), outQ(outQ), stickcore(false) {}
  // sticking constructor
  Stage(std::string name, std::function<Out(In)> f, aqueue<In>& inQ, aqueue<Out>& outQ, int coreno):
    name(name), f(f), inQ(inQ), outQ(outQ), coreno(coreno), stickcore(true) { }

  // method starting the compute thread
  std::thread * run() {
    auto body = [&] () {

      int cn; 
      if(tracecores) {
	cn = sched_getcpu();
	std::cout << "Stage " << name << " running on core " << cn << std::endl;
      }
      auto t = inQ.pop();
      while(t != INTEOS) {
	int cnn ;
	if(tracecores) {
	  cnn = sched_getcpu();
	  if(cnn != cn) {
	    std::cout << "Stage " << name << " moved " << cn << " >> " << cnn << std::endl;
	    cn = cnn;
	  }
	}
	auto r = f(t);
	// std::this_thread::sleep_for(delay);
	active_delay(delay.count());
	// cnn = sched_getcpu();
	// if(cnn != cn && tracecores) {
	//   std::cout << "Stage " << name << " moved " << cn << " >> " << cnn << std::endl;
	// }
	outQ.push(r);
	t = inQ.pop();
      }
      outQ.push(INTEOS);
      return;
    };
    
    auto tid = new std::thread(body);
    if(stickcore) 
      stickCurrentThread(name,tid,coreno);
    return tid;
  }
};

// class Source wrapper : the code is embedded in the body auto function ...
// provides a stream of Out items
// ended by and INTEOS item ...

template<typename Out> class Source {
private:
  aqueue<Out>& outQ;
  int m;
  int coreno;
  bool stickcore; 
  
public:
  Source(int m, aqueue<Out>& outQ):m(m), outQ(outQ), stickcore(false) {}
  Source(int m, aqueue<Out>& outQ, int coreno):m(m), outQ(outQ), stickcore(true), coreno(coreno) {}
  
  std::thread * run()  {
    
    auto body = [&]() {
      int cn;
      if(tracecores) {
	cn  = sched_getcpu();
	std::cout << "Source running on core " << cn << std::endl;
      }
      int i = 0;
      while(i<m) {
	int cnn;
	if(cnn != cn && tracecores) {
	  cnn  = sched_getcpu();
	  std::cout << "Stage Source moved " << cn << " >> " << cnn << std::endl;
	  cn = cnn;
	}
	// std::this_thread::sleep_for(delay);
	active_delay(delay.count());
	outQ.push(i);
	i++;
      }
      outQ.push(INTEOS);
      return;
    };
    
    auto tid = new std::thread(body);
    if(stickcore) {
      stickCurrentThread("Source",tid,coreno);
    }
    return tid;
  }
};

// class Drain wrapper: absorbes a In item stream
// reads and prints input stream items up to INTEOS
template<typename In> class Drain {
private:
  aqueue<In>& inQ;
  bool stickcore;
  int coreno;
  
public:
  Drain(aqueue<In>& inQ):inQ(inQ),stickcore(false) {}
  Drain(aqueue<In>& inQ, int coreno):inQ(inQ), stickcore(true),coreno(coreno) {}
  
  std::thread * run() {
    
    auto body = [&] () {
      int cn;
      if(tracecores) {
	cn  = sched_getcpu();
	std::cout << "Drain running on core " << cn << std::endl;
      }
      auto t = inQ.pop();
      while(t != INTEOS) {
	int cnn;
	if(cnn != cn && tracecores) {
	  cnn  = sched_getcpu();
	  std::cout << "Stage Drain moved " << cn << " >> " << cnn << std::endl;
	  cn = cnn;
	}
	// uncomment this to check the results
	//std::cout << "Drain received " << t << std::endl;
	// std::this_thread::sleep_for(delay);
	active_delay(delay.count());
	t = inQ.pop();
      }
      return;
    };
    
    auto tid = new std::thread(body);
    if(stickcore) {
      stickCurrentThread("Drain",tid,coreno);
    }
    return tid;
  }
};



int main(int argc, char * argv[])  {
  // parse command line parameters
  if(argc == 1) {
    std::cout << "Usage is: " << argv[0] << " m delay [c|f corno*]" << std::endl;
    return(0);
  }
  
  int m = atoi(argv[1]);
  int d = atoi(argv[2]);
  if(argc >= 4) {
    switch(argv[3][0]) {
    case 'c': {
      std::cout << "tracecores" << std::endl;
      tracecores = true;
      break;
    }
    case 'f': {
      std::cout << "fixthreads" << std::endl;
      fixthreads = true;
      tracecores = true;
      break;
    }
    }
  }
    
  delay = std::chrono::microseconds(d);

  // functions to be computed in the three stages
  auto f = [] (int i) { return(++i); };
  auto g = [] (int i) { return(i*i); };
  auto h = [] (int i) { return(--i); };

  {
    utimer q("      total");

    // declare communication queues
    aqueue<int> q1("input");
    aqueue<int> q2("f(x)");
    aqueue<int> q3("g(f(x))");
    aqueue<int> q4("h(g(f(x)))");;
    
    // declare stages in the pipeline
    Source<int>    s1 = (fixthreads ? Source<int>(m, q1, atoi(argv[4])) : Source<int>(m, q1));
    Stage<int,int> s2 = (fixthreads ? Stage<int,int>("sf", f, q1,q2, atoi(argv[5])) : Stage<int,int>("sf", f, q1,q2));
    Stage<int,int> s3 = (fixthreads ? Stage<int,int>("sg", g, q2,q3, atoi(argv[6])) :  Stage<int,int>("sg", g, q2,q3));
    Stage<int,int> s4 = (fixthreads ? Stage<int,int>("sh", h, q3,q4, atoi(argv[7])) : Stage<int,int>("sh", h, q3,q4));
    Drain<int>     s5 = (fixthreads ? Drain<int>(q4, atoi(argv[8])) : Drain<int>(q4));

    
    {
      utimer u("       exec");

      // start pipeline stages
      std::thread * t1 = s1.run(); 
      std::thread * t2 = s2.run();
      std::thread * t3 = s3.run();
      std::thread * t4 = s4.run();
      std::thread * t5 = s5.run();

      // then await termination of each of them 
      t1->join(); delete(t1);
      t2->join(); delete(t2);
      t3->join(); delete(t3);
      t4->join(); delete(t4);
      t5->join(); delete(t5);
    }

    // times needed to synchronize a queue access and to start/join a thread
    auto tq  =   11us;
    auto tth =   88us;

    // compute and print ideal completion times ... (for comparison)
    std::cout << "Ideal completion time = "
	      << m * delay.count() << " usec" << std::endl;
    std::cout << "More realisticallY    = "
	      << m * delay.count() + 5 * tth.count() << std::endl;
  }
  // game over
  return 0; 
}
