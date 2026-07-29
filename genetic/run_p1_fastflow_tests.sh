#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

# The bundled FastFlow release emits warnings under the project's warning
# policy. Its runtime integration is tested separately from project-owned
# warning-clean native code.
c++ -std=c++17 -O0 -g -pthread \
  -Iinclude \
  tests/p1_equivalence_tests.cpp \
  -o build/p1_fastflow_equivalence_tests

./build/p1_fastflow_equivalence_tests
