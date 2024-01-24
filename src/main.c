#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "bridge.h"

#define p_dir(d) (d == TO_NORWICH) ? "to norwich" : "to hanover"
#define MAX_CHILDREN 30
#define MAX_CARS 3

bridge_t *bridge;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvars[2] = {PTHREAD_COND_INITIALIZER}; /* indexed by dir */
int max_cars;

static void random_sleep(int min, int max);
static void random_interleaving(int thread_count);
static void all_same_dir(direction dir, int thread_count);
static void alternate_direction(int thread_count);

/*
 * ArriveBridge - car arrives to bridge and has to wait
 *
 * what it does:
 *  1) acquires the lock
 *  2) incrememnts # of cars waiting
 *  3) waits for cvar (note: there is a cvar for each direction)
 *      condition := ~ [ num_cars == 0 || ( dir == bridge_dir
 *                      && cars < MAX_CARS ]
 */
void ArriveBridge(direction dir) {
  pthread_mutex_lock(bridge->mutex);
#ifdef DEBUG
  printf("--Arriving at bridge heading %s\n", p_dir(dir));
#endif
  bridge->n_cars_waiting[dir]++;
  while (!(bridge->n_cars_on == 0 ||
           (bridge->n_cars_on < max_cars && bridge->cur_dir == dir))) {
#ifdef DEBUG
    printf("  Car waits before heading to %s ", p_dir(dir));
    if (bridge->cur_dir != dir && bridge->n_cars_on == max_cars)
      printf("because bridge is full and traffic is in wrong direction\n");
    else if (bridge->n_cars_on == max_cars)
      printf("because the bridge is full\n");
    else
      printf("because the traffic is in wrong direction\n");
#endif
    pthread_cond_wait(&cvars[dir], bridge->mutex);
  }
#ifdef DEBUG
  printf("  Car wakes up heading %s\n", p_dir(dir));
#endif
}
/*
 * OnBridge - handles when the car enters the bridge
 *
 * what it does:
 *  1) updates bridge state
 *  2) prints bridge state
 *  3) releases the lock
 *  4) sleeps for random time to simulate driving across bridge
 */
void OnBridge(direction dir) {
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
 *  3)  if bridge not empty: first signals cars going same direction up to
 * max_cars of those are waiting
 *      else: signal cars going in opposite direction up to max_cars
 *  4) give up the lock
 */
void ExitBridge(direction dir) {
  pthread_mutex_lock(bridge->mutex);
  bridge->n_cars_on--; /* get off the bridge, updating bridge state */
#ifdef DEBUG
  printf("    Exiting...n_cars_on = %d\n", bridge->n_cars_on);
#endif
  if (bridge->n_cars_on > 0 &&
      bridge->n_cars_waiting[dir] > 0) { /* bridge is not empty */
    /* signal cars going in same dir until bridge is full */
    for (int i = 0;
         i < max_cars - bridge->n_cars_on && i < bridge->n_cars_waiting[dir];
         i++) {
      pthread_cond_signal(&cvars[dir]);
#ifdef DEBUG
      printf("    Waking one thread same direction\n");
#endif
    }
  } else if (bridge->n_cars_on > 0) {
    /* don't signal */
#ifdef DEBUG
    printf("    No threads to wake up!\n");
#endif
  } else { /* the bridge is empty */
    /* let the opposite direction go first */
    int i = 0;
    for (; i < max_cars && i < bridge->n_cars_waiting[!dir]; i++) {
      pthread_cond_signal(&cvars[!dir]);
#ifdef DEBUG
      printf("    Waking one thread opposite direction\n");
#endif
    }

    if (i == 0) { // there were none waiting
      for (; i < max_cars && i < bridge->n_cars_waiting[dir]; i++) {
        pthread_cond_signal(&cvars[dir]);
#ifdef DEBUG
        printf("    Waking one thread same direction\n");
#endif
      }
    }
    if (i == 0) {
#ifdef DEBUG
      printf("    No threads to wake up!\n");
#endif
    }
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
  /* thread still holds the lock here */
  OnBridge(dir);

  ExitBridge(dir);

  /* exit the thread */
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  printf("Starting Bridge Simulation!\n");

  /* seed the RNG */
  srand(time(NULL));

  /* create bridge */
  bridge = bridge_init(&mutex);

  if (argc == 4 && strlen(argv[1]) == 1 && isdigit(argv[1][0])) {
    max_cars = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    switch (atoi(argv[1])) {
    case 0:
      random_interleaving(num_threads);
      break;
    case 1:
      all_same_dir(TO_HANOVER, num_threads);
      break;
    case 2:
      all_same_dir(TO_NORWICH, num_threads);
      break;
    case 3:
      alternate_direction(num_threads);
      break;
    default:
      fprintf(stderr, "Invalid MODE\n");
      break;
    }
  } else {
    fprintf(stderr, "usage: %s MODE MAX_CARS MAX_THREADS\n", argv[0]);
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

/*
 * random_interleaving - cars randomly arrive at both sides
 */
static void random_interleaving(int thread_count) {
  /* array of threads */
  pthread_t children[thread_count];

  /* creating the threads */
  for (int i = 0; i < thread_count; i++) {
    int rand_dir = rand() & 1;
    pthread_create(&children[i], NULL, one_vehicle, (void *)rand_dir);
    usleep(500000);
  }

  /* waiting on threads to join */
  for (int i = 0; i < thread_count; i++) {
    pthread_join(children[i], NULL);
  }
}

/*
 * all_same_direction - sends cars all in direction dir
 */
static void all_same_dir(direction dir, int thread_count) {
  /* array of threads */
  pthread_t cars[thread_count];

  int i;
  /* create threads */
  for (i = 0; i < thread_count; i++) {
    pthread_create(&cars[i], NULL, one_vehicle, (void *)dir);
  }

  /* weait for threads to join */
  for (i = 0; i < thread_count; i++) {
    pthread_join(cars[i], NULL);
  }
}

static void alternate_direction(int thread_count) {
  pthread_t cars[thread_count];

  int i;
  /* create threads */
  for (i = 0; i < thread_count; i++) {
    if (i % 2)
      pthread_create(&cars[i], NULL, one_vehicle, (void *)TO_NORWICH);
    else
      pthread_create(&cars[i], NULL, one_vehicle, (void *)TO_HANOVER);
  }

  /* weait for threads to join */
  for (i = 0; i < thread_count; i++) {
    pthread_join(cars[i], NULL);
  }
}
