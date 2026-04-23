#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "../include/timer.h"

// controllo se altro processo sia vivo ogni 4 secondi
#define FREQ_WTD 4

void reportWatchlion() {
   // per ogni elemento nell'array condiviso guardo se
   // A[i], dove A è elemento nell'array condiviso tra processi e i posizione i-esima nel processo con un determinato pid
   // A[i] = 1 --> vivo, A[0] = 0 --> morto, decido cosa fare in base ai flag che l'utente imposta
   // ad esempio salvo in un altro array questa informazione
   // INFO[i] --> se INFO[1] = 1 --> rianimo --> INFO[i] = 2 --> nulla, ecc
   // decido questo parametro grazie a *info
}

// ricevo solo SIGUSR1
void setMaskWatchlion(sigset_t *mask) {
   sigfillset(&mask);
   sigdelset(&mask, SIGUSR1);
   sigprocmask(SIG_SETMASK, mask, NULL);
}

int main(int argc, char *argv[]) {
   struct ParsedData data;
   pthread_t tid;
   int mypid;
   sigset_t mask;
   int sec = FREQ_WTD;

   mypid = getpid();

   // ricevo solo segnali di tipo SIGUSR1 e decido cosa fare quando arrivano
   setSignalWatchlion(&mask);
   
   // cerca il file di configurazione in /tmp/watchlion.init
   // se non esiste errore

   // se esiste:
   int fd_config;
   fd_config = open("/tmp/watchlion.txt", O_READ);
   if (parseFile(fd_config, &data)) {
      perror("Errore lettura file config");
      exit(0);
   }


   startProcess(&data, mypid);

   
   puts("Avvio watchlion");
   // aspetto solo il segnale da parte del timer per sbloccarmi --> SIGUSR1
   // sigsuspend(mask_tim);

   while (1) {
      puts("Avvio timer/thread");
      // ogni tot secondi mando signal al gruppo del padre
      kill(-mypid, SIGUSR1);
      pthread_create(&tid, NULL, timer_thread, &sec);
      pthread_join(tid, NULL);

      // GUARDA MEMORIA CONDIVISA TRA PADRE E FIGLI
      // array: [ 1 1 1 1 0 1 1 1 0 1 ]
      // idx:     0 1 2 3 4 5 6 7 8 9
      
      // REPORT ARRAY
      reportWatchlion();
   }

   return 0;
}
