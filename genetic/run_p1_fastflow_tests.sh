#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

c++ -std=c++17 -O1 -g -pthread \
  -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined \
  -fno-omit-frame-pointer \
  -Iinclude tests/fastflow_executor_tests.cpp \
  -o build/fastflow_executor_tests

./build/fastflow_executor_tests
