/*
 * bridge.c - implementation of bridge module
 *
 * SRB -- 24W
 */
#include "bridge.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* macro to print out direction */
#define p_dir(d) (d == TO_NORWICH) ? "to norwich" : "to hanover"

/* bridge_init */
bridge_t *bridge_init(pthread_mutex_t *mutex) {
  bridge_t *b = malloc(sizeof(bridge_t));
  b->n_cars_on = 0;
  b->cur_dir = TO_NORWICH; /* arbitrarily initialize direction */
  b->mutex = mutex;
  return b;
}

/* bridge_destroy */
void bridge_destroy(bridge_t *b) {
  if (b)
    free(b);
}

/* bridge_print */
void bridge_print(bridge_t *bridge) {
  char *s = "----------------  bridge state  ------------------\n"
            "%d car(s) on the bridge; max capacity: %d\n"
            "traffic flow goes from %s to %s\n"
            "%d cars waiting on hanover side to head to norwich\n"
            "%d cars waiting on norwich side to head to hanover\n"
            "--------------------------------------------------\n";
  printf(s, bridge->n_cars_on, max_cars, p_dir(!(bridge->cur_dir)),
         p_dir(bridge->cur_dir), bridge->n_cars_waiting[TO_NORWICH],
         bridge->n_cars_waiting[TO_HANOVER]);
}
