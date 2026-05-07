/*
   FUNZIONI RACCOLTE PER L'UTENTE
*/
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include "app.h"
#include "../include/watchlion.h"
#include "./utils/pid_hash_map.h"

// astrae il codice da parte del client per impostare sigaction su un segnale che chiama una funzione
// la funzione è personalizzabile dall'utente e può inserire più funzioni all'interno di essa (imAlive necessaria, ma anche *info) TODO
void set_watchlion(struct sigaction *sa_notify, void (* const user_handler)(int, siginfo_t *, void *), int signo, bool mod) {
   sa_notify->sa_sigaction = user_handler;
   sa_notify->sa_flags = SA_SIGINFO | SA_RESTART;
   sigfillset(&sa_notify->sa_mask); // tutti i segnali bloccati mentre sono dentro l'handler --> anche SIGUSR1
   if (mod) { // se mod == 1 allora ogni volta che arriva un segnale SIGUSR1 esco dall'handler e ci rientro
      sigdelset(&sa_notify->sa_mask, signo);
   }
   sigaction(signo, sa_notify, NULL);
}

// imposta 1 nell'array wtg di pid processi
bool im_alive(int pids_process) {
   int idx_pid;
   if ((idx_pid = get_idx_map_pid(pids_process)) == -1)
      return false;
   set_status_alive(idx_pid);
   return true;
}

bool set_behave_post_death(void (* const callback_post_death)(void *), void *context, int pids_process) {
   int idx_pid;
   if ((idx_pid = get_idx_map_pid(pids_process)) == -1)
      return false;
   set_callback_post_death(callback_post_death, context, idx_pid);
   return true;
}