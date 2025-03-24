#!/bin/bash
for n in 128 256 512 1024 2048; do
  echo "Board size: $n"
  for i in {1..5}; do
    time echo "$n 10" | ./gol
  done
  echo ""
done
