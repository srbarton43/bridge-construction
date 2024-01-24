#!/bin/bash

echo "10 Cars to Hanover; MAX_CARS=3"
./debug_bridge 1 3 10

echo "10 Cars to Norwich; MAX_CARS=5"
./debug_bridge 2 5 10

echo "10 cars heading in alternate direction; MAX_CARS=2"
./debug_bridge 3 2 10

echo "Testing Random Interleaving; MAX_CARS=5"
./debug_bridge 0 5 10
