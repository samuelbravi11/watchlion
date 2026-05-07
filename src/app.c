#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "app.h"
#include "./events/queue_sig.h"
#include "./utils/pid_hash_map.h"

struct ProcessState {
   int alive;
   void (* callback_pd)(void *);
   void *context;
};

int fd_shm_ps;   // file descriptor of shared memory process state
struct ProcessState *ptr_shm_ps;   // pointer to shared memory of ProcessState struct


int init_shm(const char *path_shm, int n_pid) {
   size_t size_ps = n_pid * sizeof (struct ProcessState);
   
   fd_shm_ps = shm_open(path_shm, O_CREAT | O_RDWR, 0666);
   if (fd_shm_ps == -1)
      return false;
   if (ftruncate(fd_shm_ps, size_ps) == -1)
      return false;
   ptr_shm_ps = mmap(NULL, size_ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_ps, 0);
   if (ptr_shm_ps == MAP_FAILED)
      return false;

   return true;
}

int init_pid_hash_map(pid_t *pids_process, size_t n_pid) {
   clear_map_pid();
   for (size_t i = 0; i < n_pid; i++) {
      if (!insert_map_pid(pids_process[i]))
         return -1;
   }
   return 0;
}

pid_t *start_all_process(struct ParsedData *data) {
   int *pids_process = calloc(data->exe.count_exe, sizeof (int));
   if (!pids_process) {
      perror("Errore allocazione memoria pids_process");
      return 0;
   }
   // avvio un processo per figlio
   for (int i = 0; i < data->exe.count_exe; i++) {
      pid_t child_pid = fork();
      if (child_pid < 0) {
         perror("Errore fork");
         return NULL;
      } else if (child_pid == 0) {
         execl(data->exe.path_exe[i], data->exe.name_exe[i], NULL);
         exit(child_pid);
      }
      pids_process[i] = child_pid;
   }

   return pids_process;
}

void check_status(pid_t *pids_process, int n_pid) {
   for (int i = 0; i < n_pid; i++) {
      struct ProcessStatus *ps = malloc(sizeof (struct ProcessStatus));
      if (!ps) return;
      int idx_process = get_idx_map_pid(pids_process[i]);

      if (ptr_shm_ps[idx_process].alive) {
         ps->pid_process = pids_process[i];
         ps->warned = 1;
         enqueue_process_status(ps);   // update reply count
         printf("PID:%d\tVIVO", pids_process[i]);
         ptr_shm_ps[idx_process].alive = 0;
      } else {
         ps->pid_process = pids_process[i];
         ps->warned = 0;
         enqueue_process_status(ps);   // update noreply count
         printf("PID:%d\tMORTO", pids_process[i]);
         // func post death
         if (ptr_shm_ps[i].callback_pd) {
            ptr_shm_ps[i].callback_pd(ptr_shm_ps[i].context);
         }
      }
   }
}

void set_status_alive(int idx_pid) {
   ptr_shm_ps[idx_pid].alive = 1;
}

void set_callback_post_death(void (* const callback)(void *), void *context, int idx_pid) {
   ptr_shm_ps[idx_pid].callback_pd = callback;
   ptr_shm_ps[idx_pid].context = context;
}