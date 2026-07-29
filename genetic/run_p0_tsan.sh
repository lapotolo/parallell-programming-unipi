#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

c++ -std=c++17 -O1 -g -pthread \
  -Wall -Wextra -Wpedantic \
  -fsanitize=thread \
  -fno-omit-frame-pointer \
  -Iinclude tests/p0_tests.cpp -o build/p0_tests_tsan

./build/p0_tests_tsan
