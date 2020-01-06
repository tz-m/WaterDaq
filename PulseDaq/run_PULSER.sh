#!/bin/bash

for i in `seq 1 2000`;
do
    echo "$i"
    ./PulseDaq -n 3e5 -d 0 -i 100 -config config_BOTH.txt
    sleep 60
done
