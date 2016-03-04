#!/bin/sh

GAME_FILE=$1
OUTPUT_FILE=$2
SECONDS_TO_RUN=$3

./bin/cadiaplayer "${GAME_FILE}" "${OUTPUT_FILE}" "${SECONDS_TO_RUN}"
