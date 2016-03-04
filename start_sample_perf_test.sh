#!/bin/sh

# Account for the fact that start_perf_test will run these from the
# bin directory... In reality we use absolute paths
GAME_FILE="../sample_game.kif"
OUTPUT_FILE="../output.out"
SECONDS_TO_RUN=2

./start_perf_test.sh $GAME_FILE $OUTPUT_FILE $SECONDS_TO_RUN
