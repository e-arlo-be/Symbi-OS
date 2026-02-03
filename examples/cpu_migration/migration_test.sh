#!/bin/bash

PROGRAM="./main"
shift

# Start program pinned to CPU 0
taskset -c 0 $PROGRAM "$@" &
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