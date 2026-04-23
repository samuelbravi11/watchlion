/*
   FUNZIONI RACCOLTE PER L'UTENTE
*/
#include "../include/watchdog.h"

// memoria condivisa provvisoria
extern int *S; // Status
extern ()(* const BD)()[]; // Before Death
extern ()(* const AD)()[]; // After Death
extern char *args_BD[];  // argomenti da passare alla BD[i] func
extern char *args_AD[];  // argomenti da passare alla AD[i] func

// astrae il codice da parte del client per impostare sigaction su un segnale che chiama una funzione
// la funzione è personalizzabile dall'utente e può inserire più funzioni all'interno di essa (imAlive necessaria, ma anche *info) TODO
void setWatchlion(struct sigaction *sa, (void)(* const f)(int, sigset_t, void *), int signo, bool mod) {
   sa->sa_handler = f;
   sa->sa_flags = SA_SIGINFO | SA_RESTART;
   sigfillset(&sa->sa_mask); // tutti i segnali bloccati mentre sono dentro l'handler --> anche SIGUSR1
   if (mod) { // se mod == 1 allora ogni volta che arriva un segnale SIGUSR1 esco dall'handler e ci rientro
      sigdelset(signo, &sa->sa_mask);
   }
   sigaction(signo, sa, NULL);
}

// imposta 1 nell'array wtg di pid processi
int imAlive(int pid_process) {
   // nella memoria condivisa scrivo: A[pid_process] = 1
}


bool setBehavePostDead(()(* const f)(char *args[]), char *args[]) {
   // nella memoria condivisa scrivo cosa fare se il processo muore
   // F[pid_process] = 1 ad esempio dice di riavviare se processo non risponde
}


bool setBehaveBeforeCheck(()(* const f)(), char *args[]) {
   // stessa cosa di sopra ma funzione che viene fatta prima di controllare A[0 ... n]
}