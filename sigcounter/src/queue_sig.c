#include <stdlib.h>
#include "../include/queue_sig.h"

struct Queue *q;

bool queue_init(void) {
   q = malloc(sizeof (struct Queue));
   if (!q)
      return false;
   q->elem_queue = calloc(QUEUE_SIZE, sizeof (struct Elem));
   if (!q->elem_queue)
      return false;
   q->head = 0;
   q->tail = 0;
   return true;
}

static int queue_full(void) {
   return ((q->head + 1) % QUEUE_SIZE) == q->tail;
}

static int queue_empty(void) {
   return q->head == q->tail;
}

bool enqueue_elem(struct Elem *e) {
   if (queue_full()) {
      return false;
   }
   q->elem_queue[q->head] = e;
   q->head = (q->head + 1) % QUEUE_SIZE;
   return true;
}

struct Elem *dequeue_elem(void) {
   if (queue_empty()) {
      return 0;
   }
   struct Elem *e = q->elem_queue[q->tail];
   q->tail = (q->tail + 1) % QUEUE_SIZE;
   return e;
}