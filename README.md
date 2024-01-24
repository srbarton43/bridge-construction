# bridge-construction

This project uses threads to simulate a one-way bridge spanning the Connecticut River.
It uses only "pure" thread operations provided by the pthread library, eg. Condition Variables and Locks.

example output of bridge state:

```
    ---------  BRIDGE  ----------
    N 021       98/99  -->  000 H
    -----------------------------
```

## Usage

Compile the project with `make`

Run the program, `simulate_bridge`, with:

```bash
./simulate_bridge MODE MAX_CARS MAX_THREADS
```

Where:
- MODE          := [0,4] and represents interleaving mode
    - 0 := random interleaving
    - 1 := all cars head to hanover
    - 2 := all cars head to norwich
    - 3 := calls alternate in direction starting with heading to norwich
- MAX_CARS      := max number of cars the bridge can hold
- MAX_THREADS   := max number of threads that the program creates

For seeing verbose output, build the project with `make debug`.

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

There is a chance, due to relative interleaving which I saw in testing, for a car exiting the bridge to wake up just after another car has signaled, making it signal the same car(s) again even though bridge state has not changed.
I observed that this interleaving case was quite rare, so it should not affect performance much.
This example is because pthreads are 'mesa-style' and thus, more importantly, with our use of the while loop the program is still safe since the mis-woken thread will check the condition and go back to sleep.

## Testing

There are a variety of tests that the simulation can perform using CLI arguments.
For MODE 0, the program randomly assigns each thread a direction, performing a test on random interleaving.
In MODE 1, cars only head to hanover, and in MODE 2, cars only head to norwich.
And in MODE 3, cars arrive perfectly alternating in direction, testing for alternate interleavings.
To test, I ran the `testing.sh` to test each different mode. 
Furthermore, I ran MODE 0 many times to witness whether expected behavior was upheld.

For each test, I compiled the program with the debug flag enabled for a more verbose display of each thread.
After no bugs were exposed through testing, I further looked into the logic of the program to double check that the design made sense.

See `testing.out` for a sample verbose (DEBUG) output of all tests.
