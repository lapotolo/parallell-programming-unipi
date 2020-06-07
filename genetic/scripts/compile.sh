#!/bin/bash

# possible improvement take arguments seq par ff all

cd /home/lapo/Documenti/repo/parallell-programming-unipi/genetic
rm -r build
mkdir -p build

echo "Compiling..."

export FF_ROOT=/home/lapo/opt/fastflow

#if [[ -n "$name" ]]; then
#log_file="Logone.txt"
#name=$1


echo "Sequential version compilation took:"
time g++ -O3 -finline-functions -std=c++17 -o ./build/seq ./src/genetic_tsp_seq.cpp

echo "Parallel version (c++ threads) compilation took:"
time g++ -O3 -finline-functions -std=c++17 -pthread -o ./build/par ./src/genetic_tsp_par.cpp

#echo "Parallel version (FastFlow) compilation took:"
#time g++ -O3 -finline-functions -Wall -Wextra -pedantic -std=c++17 -pthread -I$FF_ROOT -o ./build/ff ./src/genetic_tsp_ff.cpp

code=$?

echo ""
echo "Generated binaries! (return code ${code})"
echo "Executable in ./build"

echo ""
echo "Use command ./scripts/run.sh to run experiments using some default configurations."

echo ""
#./bin/benchmark --help