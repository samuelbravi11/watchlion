#include <stdio.h>
#include "../include/timer.h"

void *timer_thread(void *arg) {
   int seconds = *(int *)arg;
   sleep(seconds);
   timer_elapsed();
   return 0;
}

static void timer_elapsed(void) {
   printf("Timer scaduto!\n");
   // invia sig a tutti
   // chi non risponde
}