#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "./include/timer.h"

// controllo se altro processo sia vivo ogni 4 secondi
#define FREQ_WTD 4
#define FILE_CONFIG "/etc/watchlion.conf"
#define PATH_SIGCOUNTER "./sigcounter/sigcounter"
#define NAME_EXE_SIGCOUNTER "sigcounter"

int sc_pid;
int n_pid;

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
   int my_pid;
   sigset_t mask;
   int sec = FREQ_WTD;

   my_pid = getpid();

   // ricevo solo segnali di tipo SIGUSR1 e decido cosa fare quando arrivano
   setSignalWatchlion(&mask);
   
   // cerca il file di configurazione in /etc/watchlion.init
   if (access(FILE_CONFIG, R_OK) != 0) {
      // se non esiste errore
      perror("file di configurazione inesistente");
      puts("path dove mettere il file: /etc/watchlion.conf");
      exit(1);
   }
   // se esiste:
   int fd_config;
   fd_config = open(FILE_CONFIG, O_READ);
   if (parseFile(fd_config, &data)) {
      perror("Errore lettura file config");
      exit(0);
   }

   n_pid = data.exe->count_exe;
   sc_pid = startSigCounter(PATH_SIGCOUNTER, NAME_EXE_SIGCOUNTER, data.exe->count_exe, data.log->path_file);
   startProcess(&data, my_pid, sigcounter_pid);

   puts("Avvio watchlion");
   while (1) {
      puts("Mando SIGUSR1 a tutti");

      // ogni tot secondi mando signal al gruppo del padre
      kill(-my_pid, SIGUSR1);
      pthread_create(&tid, NULL, timer_thread, &sec);
      checkStatus(); // TODO: crea thread che lo fa
      pthread_join(tid, NULL);

      // GUARDA MEMORIA CONDIVISA TRA PADRE E FIGLI
      // array: [ 1 1 1 1 0 1 1 1 0 1 ]
      // idx:     0 1 2 3 4 5 6 7 8 9
   }
   // REPORT ARRAY ALLA FINE
   reportWatchlion();

   return 0;
}
