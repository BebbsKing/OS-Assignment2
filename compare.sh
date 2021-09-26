#!/bin/bash

if [[ $# -ne 1 ]]; then
    test="data_1111.txt"
else
    test=$1
fi

if [[ $test != "inputs/"* ]]; then
    test=$(echo "inputs/${test}")
fi

baseline=$(echo "outputs/baseline_$(echo $test | cut -d "_" -f 2)")
scheduler=$(echo "outputs/scheduler_$(echo $test | cut -d "_" -f 2)")

g++ baseline.cpp -o bin/baseline
g++ strf_scheduler.cpp -o bin/scheduler
g++ compare_stats.cpp -o bin/compare_stats

./bin/baseline $test $baseline
./bin/scheduler $test $scheduler

./bin/compare_stats $test $baseline $scheduler