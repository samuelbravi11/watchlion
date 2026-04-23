#ifndef QUEUE_SIG_H
#define QUEUE_SIG_H

#include "elem.h"
#include <stdbool.h>

#define QUEUE_SIZE 10

struct Queue {
   struct Elem **elem_queue;
   int num_elem;
   int head;
   int tail;
};

extern struct Queue *q;

bool queue_init(void);
bool enqueue_elem(struct Elem *e);
struct Elem *dequeue_elem(void);

#endif