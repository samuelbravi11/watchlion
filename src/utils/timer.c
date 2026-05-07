#include <stdio.h>
#include <unistd.h>
#include "timer.h"

static void timer_elapsed(void) {
   printf("Timer scaduto!\n");
}

void *timer_thread(void *arg) {
   int seconds = *(int *)arg;
   sleep(seconds);
   timer_elapsed();
   return 0;
}