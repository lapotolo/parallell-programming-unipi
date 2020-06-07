# Genetic Algorithm for Travelling Salesman Problem 
This repository contains three implementations of a genetic algorithm to approximately solve the travelling salesman problem (TSP). These implementations consist in:
 - a sequential version.
 - a multithreaded c++ version.
 - a multithreaded version relying on [FastFlow](https://github.com/fastflow/fastflow).

Use `./compile.sh` to produce executables for the three versions. The newly produced binaries are in `./build/`.

Use `./run.sh` to run the three versions of the program using some default configurations. The results can be found in `./results/`. Files' filenames in `results/` encodes the parameters used to get the results written in the file. Each file contains one entry per line corresponding to the completition time.





