#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "bridge.h"

#define p_dir(d) (d == TO_NORWICH) ? "to norwich" : "to hanover"
#define MAX_CHILDREN 30

bridge_t *bridge;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvars[2] = {PTHREAD_COND_INITIALIZER}; /* indexed by dir */

static void random_sleep(int min, int max);

/*
 * ArriveBridge - car arrives to bridge and has to wait
 *
 * what it does:
 *  1) acquires the lock
 *  2) incrememnts # of cars waiting
 *  3) waits for cvar (note: there is a cvar for each direction)
 *      proposition := cars < MAX_CARS && traffic dir == dir
 *                      || cars == 0
 *  4) release lock
 */
void ArriveBridge(direction dir) {
  // printf("Arriving at bridge heading %s\n", p_dir(dir));
  pthread_mutex_lock(bridge->mutex);
  // printf("debug arrive: dir=%d\n", dir);
  bridge->n_cars_waiting[dir]++;
  while (bridge->n_cars_on >= MAX_CARS ||
         (bridge->n_cars_on != 0 && dir != bridge->cur_dir)) {
    pthread_cond_wait(&cvars[dir], bridge->mutex);
  }
  pthread_mutex_unlock(bridge->mutex);
}
/*
 * OnBridge - handles when the car enters the bridge
 *
 * what it does:
 *  1) acquires the lock
 *  2) updates bridge state
 *  3) prints bridge state
 *  4) releases the lock
 *  5) sleeps for random time to simulate driving across bridge
 */
void OnBridge(direction dir) {
  pthread_mutex_lock(bridge->mutex);
  bridge->n_cars_waiting[dir]--;
  bridge->n_cars_on++;
  bridge->cur_dir = dir;
  bridge_print(bridge);
  pthread_mutex_unlock(bridge->mutex);
  random_sleep(2, 5);
}
/*
 * ExitBridge - the "car" exits the bridge
 *
 * what it does:
 *  1) acquires the lock
 *  2) decrements number of cars on bridge
 *  3) if bridge not empty: first signals cars going same direction up to
 * MAX_CARS who are waiting else: signal cars going in opposite direction up to
 * MAX_CARS 4) give up the lock
 */
void ExitBridge(direction dir) {
  pthread_mutex_lock(bridge->mutex);
  bridge->n_cars_on--;         /* get off the bridge, updating bridge state */
  if (bridge->n_cars_on > 0) { /* bridge is not empty */
    /* signal cars going in same dir until bridge is full */
    for (int i = 0; i < MAX_CARS - bridge->n_cars_on; i++)
      pthread_cond_signal(&cvars[bridge->cur_dir]);
  } else { /* the bridge is empty */
    /* let the opposite direction go first */
    int i = 0;
    for (; i < MAX_CARS; i++)
      pthread_cond_signal(&cvars[!dir]);
    for (; i < MAX_CARS; i++)
      pthread_cond_signal(&cvars[dir]);
  }
  pthread_mutex_unlock(bridge->mutex);
}

/*
 * one_vehicle - simulates lifetime of car
 *
 * what it does:
 *  1) arrives at bridge and waits until it can go
 *  2) enters bridge taking a non-zero time to cross
 *  3) exits bridge and alerts proper cars they can try to cross
 */
void *one_vehicle(void *vargp) {
  direction dir = (direction)vargp;
  ArriveBridge(dir);

  OnBridge(dir);

  ExitBridge(dir);

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  printf("Starting Bridge Simulation!\n");

  /* seed the RNG */
  srand(time(NULL));

  /* create bridge */
  bridge = bridge_init(&mutex);

  /* array of threads */
  pthread_t children[MAX_CHILDREN];

  /* creating the threads */
  for (int i = 0; i < MAX_CHILDREN; i++) {
    int rand_dir = rand() & 1;
    pthread_create(&children[i], NULL, one_vehicle, (void *)rand_dir);
  }

  /* waiting on threads to join */
  for (int i = 0; i < MAX_CHILDREN; i++) {
    pthread_join(children[i], NULL);
  }

  bridge_destroy(bridge);
  return 0;
}

/*
 * random_sleep - sleep for random seconds
 *
 * borrowed from sem_example2.c
 */
void random_sleep(int min, int max) {

  int rn;
  int delay;
  int delta;
  int mod;

  getrandom(&rn, sizeof(int), 0);

  delta = max - min;
  mod = abs(rn) % delta;
  delay = min + mod;
  sleep(delay);
}
