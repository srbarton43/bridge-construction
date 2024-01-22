/* vehicle.h - header file for vehicle module
 *
 * SRB -- 24W
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H

#include <pthread.h>

#define MAX_CARS 3

typedef enum { TO_NORWICH, TO_HANOVER } direction;

typedef struct bridge {
  pthread_mutex_t *mutex;
  int n_cars_on;         /* number of cars on bridge currently */
  int n_cars_waiting[2]; /* number of cars on waiting on each side */
                         /* indexed by cur_dir */
  direction cur_dir;     /* current direction of traffic */
} bridge_t;

bridge_t *bridge_init(pthread_mutex_t *mutex);

void bridge_destroy(bridge_t *b);

void bridge_print(bridge_t *bridge);

#endif
