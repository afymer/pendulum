#!/usr/bin/env sh
set -e

for arg in "$@"; do declare $arg='1'; done

mkdir -p build

cd build
clang -std=c23 -O0 -g -Wall -Wextra -Wpedantic -I../src \
      ../src/pendulum/pendulum_main.c -o pendulum
cd ..

if [[ "$run" == "1" ]]; then
    ./build/pendulum
fi
