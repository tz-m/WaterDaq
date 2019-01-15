#!/bin/bash

for i in `seq 1 2000`;
do
    echo "$i"
    ./PulseDaq -n 2e5 -d 0 -i 100 -config config_CH4.txt
    ./PulseDaq -n 2e5 -d 0 -i 100 -config config_CH5.txt
done
