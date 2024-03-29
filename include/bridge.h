/* vehicle.h - header file for vehicle module
 *
 * contains external-facing struct for bridge
 *  and methods to initialize the struct/print state
 * SRB -- 24W
 */
#ifndef __VEHICLE_H
#define __VEHICLE_H
#include <pthread.h> /* global variable to represent max cars on bridge */
extern int max_cars;

typedef enum { TO_NORWICH, TO_HANOVER } direction;

typedef struct bridge {
  pthread_mutex_t *mutex;
  int n_cars_on;         /* number of cars on bridge currently */
  int n_cars_waiting[2]; /* number of cars on waiting on each side */
                         /* indexed by direction */
  direction cur_dir;     /* current direction of traffic */
} bridge_t;

/*
 * bridge_init - initialize bridge struct
 *
 * params:
 *    mutex := the bridge's lock
 *
 * returns:
 *    pointer to bridge struct
 *
 * caller must:
 *    call bridge_destroy on pointer to clean up
 */
bridge_t *bridge_init(pthread_mutex_t *mutex);

/*
 * bridge_destroy - clean up the bridge
 *
 * params:
 *    b := pointer to bridge struct
 */
void bridge_destroy(bridge_t *b);

/*
 * bridge_print - prints out bridge state
 *
 * does such in this format:
 *    N XXX [<--] AA/BB [-->] YYY H
 *  where XXX and YYY represent cars waiting at respective sides
 *  and, AA = cars on bridge and BB = max_cars
 *  and the arrow represents traffic direction
 *
 * params:
 *    bridge := pointer to bridge struct
 */
void bridge_print(bridge_t *bridge);

#endif
