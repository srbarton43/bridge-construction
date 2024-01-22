#include "bridge.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define p_dir(d) (d == TO_NORWICH) ? "to norwich" : "to hanover"

bridge_t *bridge_init(pthread_mutex_t *mutex) {
  bridge_t *b = malloc(sizeof(bridge_t));
  b->n_cars_on = 0;
  b->cur_dir = TO_NORWICH;
  b->mutex = mutex;
  return b;
}

void bridge_destroy(bridge_t *b) {
  if (b)
    free(b);
}
void bridge_print(bridge_t *bridge) {
  char *s = "-------------------------------\n"
            "%d cars of max %d on bridge heading to %s\n"
            "%d cars waiting on hanover side\n"
            "%d cars waiting on norwich side\n"
            "-------------------------------\n";
  printf(s, bridge->n_cars_on, MAX_CARS, p_dir(bridge->cur_dir),
         bridge->n_cars_waiting[TO_NORWICH],
         bridge->n_cars_waiting[TO_HANOVER]);
}
