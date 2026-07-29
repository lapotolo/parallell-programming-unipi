#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

sanitizer_flags=(
  -std=c++17
  -O1
  -g
  -pthread
  -Wall
  -Wextra
  -Wpedantic
  -fsanitize=address,undefined
  -fno-omit-frame-pointer
  -Iinclude
)

c++ "${sanitizer_flags[@]}" \
  tests/p0_tests.cpp \
  -o build/p0_tests

c++ "${sanitizer_flags[@]}" \
  -DP1_DISABLE_FASTFLOW \
  tests/p1_equivalence_tests.cpp \
  -o build/p1_native_equivalence_tests

run_asan_binary()
{
  if [[ "$(uname -s)" == "Darwin" ]]; then
    "$1"
  else
    ASAN_OPTIONS=detect_leaks=1 "$1"
  fi
}

run_asan_binary ./build/p0_tests
run_asan_binary ./build/p1_native_equivalence_tests
