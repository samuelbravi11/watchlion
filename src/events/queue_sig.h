#ifndef QUEUE_SIG_H
#define QUEUE_SIG_H

#include <stdbool.h>
#include "process_status.h"

#define QUEUE_SIZE 10

struct Queue {
   struct ProcessStatus **process_queue;
   int num_elem;
   int head;
   int tail;
};

// extern struct Queue q;

bool queue_init(void);
bool enqueue_process_status(struct ProcessStatus *ps);
struct ProcessStatus *dequeue_process_status(void);

#endif