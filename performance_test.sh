#!/bin/bash

echo "Building the project..."
cmake -B /app/build /app && make -C /app/build

echo "Running performance test..."
for i in {1..100}
do
    time sudo /app/build/locid > /dev/null
done
