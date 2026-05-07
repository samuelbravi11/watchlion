#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "app.h"
#include "utils/pid_hash_map.h"
#include "../include/watchlion.h"

static int watchlion_attached = 0;

static bool attach_from_environment(void) {
   if (watchlion_attached) {
      return true;
   }

   const char *shm_name = getenv("WATCHLION_SHM_NAME");
   const char *count_s = getenv("WATCHLION_COUNT");
   if (!shm_name) {
      shm_name = WATCHLION_DEFAULT_SHM_NAME;
   }
   if (!count_s) {
      return false;
   }

   char *end = NULL;
   unsigned long count = strtoul(count_s, &end, 10);
   if (end == count_s || count == 0) {
      return false;
   }
   if (!attach_shm(shm_name, (size_t)count)) {
      return false;
   }

   watchlion_attached = 1;
   return true;
}

static int get_index_from_environment(void) {
   const char *idx_s = getenv("WATCHLION_INDEX");
   if (!idx_s) {
      return -1;
   }
   char *end = NULL;
   long idx = strtol(idx_s, &end, 10);
   if (end == idx_s || idx < 0) {
      return -1;
   }

   return (int)idx;
}


// astrae il codice da parte del client per impostare sigaction su un segnale che chiama una funzione
// la funzione è personalizzabile dall'utente e può inserire più funzioni all'interno di essa
void set_watchlion(struct sigaction *sa_notify, void (* const user_handler)(int, siginfo_t *, void *), int signo, bool mod) {
   if (!sa_notify || !user_handler) {
      return;
   }
   sa_notify->sa_sigaction = user_handler;
   sa_notify->sa_flags = SA_SIGINFO | SA_RESTART;
   sigfillset(&sa_notify->sa_mask);

   if (mod) {  // se mod == 1 allora ogni volta che arriva un segnale SIGUSR1 esco dall'handler e ci rientro
      sigdelset(&sa_notify->sa_mask, signo);
   }
   sigaction(signo, sa_notify, NULL);
}

bool im_alive(pid_t pid_process) {
   if (!attach_from_environment()) {
      return false;
   }
   int idx_pid = get_idx_map_pid(pid_process);
   if (idx_pid == -1) {
      idx_pid = get_index_from_environment();
   }
   if (idx_pid == -1) {
      return false;
   }
   set_status_alive(idx_pid);
   return true;
}

bool set_behave_post_death(void (* const callback_post_death)(void *), void *context, pid_t pid_process) {
   if (!attach_from_environment()) {
      return false;
   }
   int idx_pid = get_idx_map_pid(pid_process);
   if (idx_pid == -1) {
      idx_pid = get_index_from_environment();
   }
   if (idx_pid == -1) {
      return false;
   }
   set_callback_post_death(callback_post_death, context, idx_pid);
   return true;
}