#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "watchlion.h"

static void handler_watchlion(int signo, siginfo_t *info, void *context) {
   (void)signo;
   (void)info;
   (void)context;

   im_alive(getpid());
}

int main(void) {
   struct sigaction sa;
   set_watchlion(&sa, handler_watchlion, SIGUSR1, true);

   while (1) {
      pause();
   }

   return 0;
}