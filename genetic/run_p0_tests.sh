#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

c++ -std=c++17 -O1 -g -pthread \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined \
  -fno-omit-frame-pointer \
  -Iinclude tests/p0_tests.cpp -o build/p0_tests

if [[ "$(uname -s)" == "Darwin" ]]; then
  ./build/p0_tests
else
  ASAN_OPTIONS=detect_leaks=1 ./build/p0_tests
fi