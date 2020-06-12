#!/bin/bash

# possible improvement: take arguments seq par ff all

rm -r build
mkdir -p build

echo "Compiling..."

export FF_ROOT=./include

echo "Sequential version compilation took:"
time g++ -O3 -finline-functions -std=c++17 -o ./build/seq ./src/genetic_tsp_seq.cpp

echo "Parallel version (c++ threads) compilation took:"
time g++ -O3 -finline-functions -std=c++17 -pthread -o ./build/par ./src/genetic_tsp_par.cpp

echo "Parallel version (FastFlow) compilation took:"
time g++ -O3 -finline-functions -std=c++17 -pthread -I$FF_ROOT -o ./build/ff ./src/genetic_tsp_ff.cpp

code=$?

echo ""
echo "Generated binaries! (return code ${code})"
echo "Executable in ./build"

echo ""
echo "Setting up the required directory structure"

rm -r results
mkdir  results
echo "Created directory results/ to store experiments' results"


echo ""
echo "Use the provided jupyter notebook to run the experiments interactively."
echo "Or use the script run.sh to run some default experiments and populate and directory ./results."