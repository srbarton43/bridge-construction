#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>

#include "bridge.h"

#define p_dir(d) (d == TO_NORWICH) ? "to norwich" : "to hanover"
#define MAX_CHILDREN 10

bridge_t *bridge;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvars[2] = {PTHREAD_COND_INITIALIZER};

void ArriveBridge(direction dir) {
  //printf("Arriving at bridge heading %s\n", p_dir(dir));
  pthread_mutex_lock(bridge->mutex);
  bridge->n_cars_waiting[dir]++;
  
  //printf("debug arriving: on bridge: %d; direction: %s; bridge direction: %s\n", bridge->n_cars_on, p_dir(dir), p_dir(bridge->cur_dir));
  while (bridge->n_cars_on >= MAX_CARS ||
         (bridge->n_cars_on != 0 && dir != bridge->cur_dir)) {
    pthread_cond_wait(&cvars[dir], bridge->mutex);
  }
  bridge->n_cars_waiting[dir]--;
  bridge->n_cars_on++;
  pthread_mutex_unlock(bridge->mutex);
}
void OnBridge(direction dir) {
  //printf("On bridge heading %s\n", p_dir(dir));
  sleep(1);
  pthread_mutex_lock(bridge->mutex);
  bridge->cur_dir = dir;
  bridge_print(bridge);
  pthread_mutex_unlock(bridge->mutex);
}
void ExitBridge(direction dir) {
  /* TODO
   *
   * if still some cars in stame direction, signal car in that direction
   *
   * else if no cars, signal both directions
   */
  pthread_mutex_lock(bridge->mutex);
  //printf("Exiting bridge heading %s\n", p_dir(dir));
  bridge->n_cars_on--;
  /* signal here */
  // TODO:
  if (bridge->n_cars_on > 0) {
    for(int i = 0; i < MAX_CARS - bridge->n_cars_on; i++) 
      pthread_cond_signal(&cvars[bridge->cur_dir]);
  } else { /* the bridge is empty */
    /* signal the longer queue? */
    for(int i = 0; i < MAX_CARS; i++)
      pthread_cond_signal(&cvars[!dir]);
    for(int i = 0; i < MAX_CARS; i++)
      pthread_cond_signal(&cvars[dir]);
  }
  pthread_mutex_unlock(bridge->mutex);
}

void *one_vehicle(void *vargp) {
  direction dir = (direction)vargp;
  ArriveBridge(dir);

  OnBridge(dir);

  ExitBridge(dir);

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  printf("Hello World!\n");
  /* init lock */
  /* create bridge */
  bridge = bridge_init(&mutex);

  bridge_print(bridge);

  /* array of threads */
  pthread_t children[MAX_CHILDREN];

  for (int i = 0; i < MAX_CHILDREN; i++) {
    printf("Creating thread %d\n", i);
    pthread_create(&children[i], NULL, one_vehicle, (void *)TO_HANOVER);
  }

  for (int i = 0; i < MAX_CHILDREN; i++) {
    pthread_join(children[i], NULL);
  }
  /*
  pthread_t t;
  int rc = pthread_create(&t, NULL, one_vehicle, (void *)TO_HANOVER);
  if (rc) {
    fprintf(stderr, "Creating thread failed\n");
    exit(1);
  }

  rc = pthread_join(t, NULL);
  if (rc) {
    fprintf(stderr, "Joining on thread failed\n");
    exit(1);
  }
  */

  bridge_print(bridge);

  bridge_destroy(bridge);
  return 0;
}
