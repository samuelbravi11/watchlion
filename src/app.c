#include app.h

// memoria condivisa provvisoria
extern bool *S; // Status
extern ()(* const BD)(char **)[]; // Before Death
extern ()(* const AD)(char **)[]; // After Death
extern char *args_BD[];  // argomenti da passare alla BD[i] func
extern char *args_AD[];  // argomenti da passare alla AD[i] func
// queste 2 extern sotto bruttissime
extern int sc_pid;
extern int n_pid;

// TODO: implement hash map --> PID programs --> idx S
int initProvvisoriaMem(struct ParsedData *pd) {
   P = calloc(pd->exe->count_exe, sizeof (int));
   S = calloc(pd->exe->count_exe, 1);
   BC = calloc(pd->exe->count_exe, sizeof (()(* const f)(char **)));
   AD = calloc(pd->exe->count_exe, sizeof (()(* const f)(char **)));
   args_BC = calloc(pd->exe->count_exe, sizeof (char *));
   args_AD = calloc(pd->exe->count_exe, sizeof (char *));

   if (!(S && BC && AD && args_BC && args_AD)) {
      perror("Errore allocazione memoria condivisa provvisoria");
      return 1;
   }
   return 0;
}

int startProcess(struct ParsedData *data, int wtd_pid, int sc_pid) {
   // avvio un processo per figlio
   for (int i = 0; i < npid; i++) {
      int pid = fork();
      if (pid < 0) {
         perror("Errore fork");
         return 1;
      } else if (pid == 0) {
         // get pid of ./a.out
         // send signal to pid of ./a.out
         execl(data.exe->path_exe[i], data.exe->name_exe[i], wtd_pid, sc_pid, NULL);
         exit(mypid);
      }
   }
   return 0;
}

int startSigCounter(char *path_exe_sigcounter, char *name_exe_sigcounter, int n_pid, char *path_logfile) {
   int pid = fork();
   if (pid < 0) {
      perror("Errore fork during startSigCounter");
      return 1;
   } else if (pid == 0) {
      execl(path_exe_sigcounter, name_exe_sigcounter, n_pid, path_logfile, NULL);
      exit(pid);
   }
   return pid;
}


void checkStatus(void) {
   for (int i = 0; i < n_pid; i++) {
      if (!S[i]) {   // se è morto
         // COUNTER DEATH
         kill(sc_pid, SIGUS2);
         // func post death
         if (AD[i]) {
            AD[i](args_AD[i]);
         }
      } else {
         kill(sc_pid, SIGUSR1);
         S[i] = 0;
      }
   }
}