#ifndef LOCKS_H
#define LOCKS_H

#include <stdbool.h>
void lock_init_mutex();
bool lock_take_mutex();
void lock_give_mutex();

#endif