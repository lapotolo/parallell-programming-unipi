#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

common_flags=(
  -std=c++17
  -O1
  -g
  -pthread
  -Wall
  -Wextra
  -Wpedantic
  -fsanitize=thread
  -fno-omit-frame-pointer
  -Iinclude
)

c++ "${common_flags[@]}" \
  tests/p0_tests.cpp \
  -o build/p0_tests_tsan

c++ "${common_flags[@]}" \
  -DP1_DISABLE_FASTFLOW \
  tests/p1_equivalence_tests.cpp \
  -o build/p1_native_equivalence_tests_tsan

./build/p0_tests_tsan
./build/p1_native_equivalence_tests_tsan
