#include <stdlib.h>
#include "queue_sig.h"

struct Queue q;

bool queue_init(void) {
   q.process_queue = calloc(QUEUE_SIZE, sizeof (struct ProcessStatus *));
   if (!q.process_queue)
      return false;
   q.head = 0;
   q.tail = 0;
   return true;
}

static int queue_full(void) {
   return ((q.head + 1) % QUEUE_SIZE) == q.tail;
}

static int queue_empty(void) {
   return q.head == q.tail;
}

bool enqueue_process_status(struct ProcessStatus *ps) {
   if (queue_full()) {
      return false;
   }
   q.process_queue[q.head] = ps;
   q.head = (q.head + 1) % QUEUE_SIZE;
   return true;
}

struct ProcessStatus *dequeue_process_status(void) {
   if (queue_empty()) {
      return 0;
   }
   struct ProcessStatus *ps = q.process_queue[q.tail];
   q.tail = (q.tail + 1) % QUEUE_SIZE;
   return ps;
}