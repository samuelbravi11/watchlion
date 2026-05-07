#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/fcntl.h>
#include "parsed_data.h"
#include "parser.h"
#include "../src/app.h"
#include "../utils/timer.h"
#include "../src/events/counter.h"
#include "../src/events/queue_sig.h"

// controllo se altro processo sia vivo ogni 4 secondi
#define FREQ_WTD 4
#define FILE_CONFIG "/etc/watchlion.conf"
#define PATH_SIGCOUNTER "./sigcounter/sigcounter"
#define NAME_EXE_SIGCOUNTER "sigcounter"

void init_sigaction(struct sigaction *sa_notify, struct sigaction *sa_report) {
   // sigusr1 usato per avvisare i child che implementeranno la loro versione di sa_notify
   sa_notify->sa_handler = SIG_IGN;
   sigaction(SIGUSR1, sa_notify, NULL);
   
   // ricevo solo SIGUSR2 per printReport
   sa_report->sa_handler = report_handler;
   sigfillset(&sa_report->sa_mask); // blocco tutte le signal mentre eseguo report_handler
   sa_report->sa_flags = 0;
   sigaction(SIGUSR2, sa_report, NULL);
}

int main(int argc, char *argv[]) {
   struct sigaction sa_notify, sa_report;
   struct ParsedData data;
   pthread_t tid;
   pid_t *pids_process, my_pid;
   int n_pid;
   int sec = FREQ_WTD;

   init_sigaction(&sa_notify, &sa_report);
   queue_init();
   my_pid = getpid();
   
   // cerca il file di configurazione in /etc/watchlion.init
   if (access(FILE_CONFIG, R_OK) != 0) {
      // se non esiste errore
      perror("file di configurazione inesistente");
      puts("path dove mettere il file: /etc/watchlion.conf");
      exit(1);
   }
   // se esiste il file di configurazione parso i dati
   int fd_config;
   fd_config = open(FILE_CONFIG, O_RDONLY);
   if (parse_file(fd_config, &data)) {
      perror("Errore lettura file config");
      exit(0);
   }

   n_pid = data.exe.count_exe;
   pids_process = start_all_process(&data);
   init_pid_hash_map(pids_process, n_pid);
   init_stats(pids_process, n_pid);
   init_shm("/watchlion", n_pid);

   puts("Avvio watchlion");
   while (1) {
      puts("Mando SIGUSR1 a tutti");

      // ogni tot secondi mando signal al gruppo del padre
      kill(-my_pid, SIGUSR1);
      pthread_create(&tid, NULL, timer_thread, &sec);
      pthread_join(tid, NULL);
      check_status(pids_process, n_pid); // TODO: crea thread che lo fa

      // GUARDA MEMORIA CONDIVISA TRA PADRE E FIGLI
      // array: [ 1 1 1 1 0 1 1 1 0 1 ]
      // idx:     0 1 2 3 4 5 6 7 8 9
   }

   return 0;
}
