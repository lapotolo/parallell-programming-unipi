#!/usr/bin/env bash
set -euo pipefail

rm -rf build
mkdir -p build results/runs

common_flags=(
  -std=c++17
  -O3
  -finline-functions
  -Wall
  -Wextra
  -Wpedantic
  -Iinclude
)

echo "Compiling sequential version..."
c++ "${common_flags[@]}" \
  src/genetic_tsp_seq.cpp \
  -o build/seq

echo "Compiling raw-thread version..."
c++ "${common_flags[@]}" -pthread \
  src/genetic_tsp_par.cpp \
  -o build/par

echo "Compiling thread-pool version..."
c++ "${common_flags[@]}" -pthread \
  src/genetic_tsp_pool.cpp \
  -o build/pool

echo "Compiling FastFlow version..."
c++ "${common_flags[@]}" -pthread \
  src/genetic_tsp_ff.cpp \
  -o build/ff

echo "Generated binaries in ./build"
echo "Benchmark results will be appended under ./results/runs"
