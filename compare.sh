#!/bin/bash

g++ baseline.cpp -o bin/baseline
g++ scheduler.cpp -o bin/scheduler
g++ compare_stats.cpp -o bin/compare_stats

./bin/baseline inputs_outputs/data_1111.txt inputs_outputs/baseline_1111.txt
./bin/scheduler inputs_outputs/data_1111.txt inputs_outputs/scheduler_1111.txt

./bin/compare_stats inputs_outputs/data_1111.txt inputs_outputs/baseline_1111.txt inputs_outputs/scheduler_1111.txt