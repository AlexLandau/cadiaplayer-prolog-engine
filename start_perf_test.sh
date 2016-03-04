#!/bin/sh

GAME_FILE=$1
OUTPUT_FILE=$2
SECONDS_TO_RUN=$3

# The code will add to the file without creating it, so rm it first.
rm "${OUTPUT_FILE}"
# The Prolog connection code requires running from the bin directory
# so we can run some additional executables.
cd bin
./cadiaplayer "${GAME_FILE}" "${OUTPUT_FILE}" "${SECONDS_TO_RUN}"
