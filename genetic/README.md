# Genetic Algorithm for Travelling Salesman Problem 
This repository contains three C++ implementations of a genetic algorithm to approximately solve the travelling salesman problem (TSP). These implementations consist in:
 - a sequential version.
 - a multithreaded c++ version.
 - a multithreaded version relying on [FastFlow](https://github.com/fastflow/fastflow).

IMPORTANT: before running the scripts assure you are positioned in `/THIS-REPO-ROOT/genetic/`.

Use `./compile.sh` to produce executables for the four versions. The newly produced binaries will be located in `./build/`.

Use `./run.sh` to run the four versions of the program using some default configurations. Results will be stored in `./results/`.

To change the default configuration you can easily modify the file `./run.sh`.

Files' filenames in `results/runs/` encodes the parameters used to get the results written in the corresponding files. Each file contains one entry per line corresponding to its relative service time.

By running the default experiments using `./run.sh` there will also be produced four more files in the folder `./results/`:
  - `t_seq.data`
  - `t_par.data`
  - `t_pool.data`
  - `t_ff.data`

The first file contains service times of a set of repeated runs of the sequential version on different instances of same size.
The other three files contain service times of the two parallel versions when run on instances of the same size but with increasing parallel degree. Each batch of experiments is repeated a fixed number of times. (`10` by default)


`t_seq.data`, `t_par.data`, `t_pool.data`, `t_ff.data` are taken in input by the python script `do_plots.py` in order to produce useful plots to understand the performances of these implementations. The chosen measures are speedup, scalability and efficiency.



