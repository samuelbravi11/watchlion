#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "app.h"
#include "events/counter.h"
#include "events/queue_sig.h"
#include "parser/parser.h"
#include "utils/timer.h"

#define FREQ_WTD 4
#define FILE_CONFIG "/etc/watchlion.conf"
#define SHM_NAME WATCHLION_DEFAULT_SHM_NAME

static void init_sigaction(struct sigaction *sa_notify, struct sigaction *sa_report) {
   sa_notify->sa_handler = SIG_IGN;
   sigemptyset(&sa_notify->sa_mask);
   sa_notify->sa_flags = 0;
   sigaction(SIGUSR1, sa_notify, NULL);

   sa_report->sa_handler = report_handler;
   sigfillset(&sa_report->sa_mask);
   sa_report->sa_flags = 0;
   sigaction(SIGUSR2, sa_report, NULL);
}

int main(void) {
   struct sigaction sa_notify, sa_report;
   struct ParsedData data;
   pthread_t tid;
   pid_t *pids_process = NULL;
   int sec = FREQ_WTD;

   init_sigaction(&sa_notify, &sa_report);

   if (!queue_init()) {
      perror("queue_init");
      exit(EXIT_FAILURE);
   }

   if (access(FILE_CONFIG, R_OK) != 0) {
      perror("file di configurazione inesistente");
      puts("path dove mettere il file: /etc/watchlion.conf");
      exit(EXIT_FAILURE);
   }

   int fd_config = open(FILE_CONFIG, O_RDONLY);
   if (fd_config == -1) {
      perror("open config");
      exit(EXIT_FAILURE);
   }

   if (parse_file(fd_config, &data) != 0) {
      perror("Errore lettura file config");
      close(fd_config);
      exit(EXIT_FAILURE);
   }
   close(fd_config);

   int n_pid = (int)data.exe.count_exe;
   if (n_pid <= 0) {
      fprintf(stderr, "Nessun eseguibile configurato\n");
      exit(EXIT_FAILURE);
   }

   shm_unlink(SHM_NAME);
   if (!init_shm(SHM_NAME, n_pid)) {
      fprintf(stderr, "Errore inizializzazione shared memory\n");
      exit(EXIT_FAILURE);
   }

   pids_process = start_all_process(&data, SHM_NAME);
   if (!pids_process) {
      fprintf(stderr, "Errore avvio processi figli\n");
      exit(EXIT_FAILURE);
   }

   if (init_pid_hash_map(pids_process, (size_t)n_pid) != 0) {
      fprintf(stderr, "Errore inizializzazione mappa PID\n");
      exit(EXIT_FAILURE);
   }

   init_stats(pids_process, n_pid);

   puts("Avvio watchlion");
   while (1) {
      puts("Mando SIGUSR1 a tutti");

      for (int i = 0; i < n_pid; i++) {
         kill(pids_process[i], SIGUSR1);
      }

      if (pthread_create(&tid, NULL, timer_thread, &sec) == 0) {
         pthread_join(tid, NULL);
      }

      check_status(pids_process, n_pid);
      check_events();
   }

   return 0;
}