#ifndef COUNTER_H
#define COUNTER_H

#include <stddef.h>
#include "elem.h"

typedef struct Counter Counter;

void init_counter(const size_t num_process);
int updateCounter(const struct Elem *e);
void printReport(void);

#endif