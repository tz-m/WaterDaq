#!/bin/bash

for i in `seq 1 2000`;
do
    echo "$i"
    time ./PulseDaq -n 2e5 -d 0 -i 100
    sleep 60
done
