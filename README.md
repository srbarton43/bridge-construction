# bridge-construction

This project uses threads to simulate a one-way bridge spanning the Connecticut River.
It uses only "pure" thread operations provided by the pthread library, eg. Condition Variables and Locks.

## Bridge Rules

Cars going in opposite directions will crash...so it is not allowed.
If there are more cars than MAX_CARS on the bridge at one, the bridge collapses, thus the number of cars must never exceed MAX_CARS.

## Safety

Each time the car arrives at the bridge, it acquires the bridge's lock so that there can be no data races on the bridge's state.
The car waits on a condition variable inside of a while loop because we are using "mesa-style" threads, so bridge rules are followed.
When printing the bridge's state, the car must hold the bridges lock, so there may be no data races there.

## Liveness

When a car arrives on the bridge, it acquires the lock, updating bridge state, and printing it.
The thread sleeps to simulate time spent driving across, but otherwise the thread does not wait, meaning that it continues straight to ExitBridge.

For the second condition, since the thread does not wait if the bridge is empty, the car will not wait forever if the bridge is empty. 

## Efficiency

When a car exits the bridge, the thread signals as many waiting cars as the bridge can hold...not just broadcasting to all waiting cars.
For example, take a car driving from hanover to norwich. 
There are two cases:
- If there are still cars on the bridge, the thread will signal (MAX_CARS - NUM_CARS_ON_BRIDGE) threads waiting on HANOVER_SIDE_CVAR.
- Else, if there are no cars on the bridge, the exiting thread signals (MAX_CARS) threads waiting on the opposite side of the bridge.
