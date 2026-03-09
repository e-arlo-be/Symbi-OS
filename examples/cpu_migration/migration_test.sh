#!/bin/bash

MODE="sleep"

#-b for busy loop, -io for block on IO
if [[ "$1" == "-b" ]]; then
    MODE="busy"
    shift
elif [[ "$1" == "-io" ]]; then
    MODE="io"
    shift
fi

PROGRAM="./main"
shift

# Start program pinned to CPU 0
taskset -c 0 "$PROGRAM" "$MODE" &
PID=$!

echo "Started $PROGRAM with PID $PID"

sleep 2

while kill -0 $PID 2>/dev/null; do
    echo "Moving to CPU 1"
    taskset -cp 1 $PID
    sleep 20

    echo "Moving to CPU 0"
    taskset -cp 0 $PID
    sleep 20
done

echo "Process exited."