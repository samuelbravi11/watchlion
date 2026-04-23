#include app.h

// memoria condivisa provvisoria
extern bool *S; // Status
extern ()(* const BD)(char **)[]; // Before Death
extern ()(* const AD)(char **)[]; // After Death
extern char *args_BD[];  // argomenti da passare alla BD[i] func
extern char *args_AD[];  // argomenti da passare alla AD[i] func

// TODO: implement hash map --> PID programs --> idx S
initProvvisoriaMem(struct ParsedData *pd) {
   S = calloc(pd->exe->count_exe, 1);
   BD = calloc(pd->exe->count_exe, sizeof (()(* const f)(char **)));
   AD = calloc(pd->exe->count_exe, sizeof (()(* const f)(char **)));
   args_BD = calloc(pd->exe->count_exe, sizeof (char *));
   args_AD = calloc(pd->exe->count_exe, sizeof (char *));
}

startProcess(struct ParsedData *data, int wtd_pid) {
   // avvio un processo per figlio
   for (int i = 0; i < npid; i++) {
      int pid = fork();
      if (pid < 0) {
         perror("Errore fork");
      } else if (pid == 0) {
         // get pid of ./a.out
         // send signal to pid of ./a.out
         execl(data.exe->path_exe[i], data.exe->name_exe[i], wtd_pid, NULL);
         exit(mypid);
      }
   }
}