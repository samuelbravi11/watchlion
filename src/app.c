#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "app.h"
#include "events/queue_sig.h"
#include "utils/pid_hash_map.h"

static int fd_shm_ps = -1;
static struct ProcessState *ptr_shm_ps = NULL;
static size_t shm_count = 0;

int init_shm(const char *path_shm, int n_pid) {
   if (!path_shm || n_pid <= 0) {
      errno = EINVAL;
      return false;
   }

   size_t size_ps = (size_t)n_pid * sizeof(struct ProcessState);

   fd_shm_ps = shm_open(path_shm, O_CREAT | O_RDWR, 0666);
   if (fd_shm_ps == -1) {
      perror("shm_open");
      return false;
   }

   if (ftruncate(fd_shm_ps, (off_t)size_ps) == -1) {
      perror("ftruncate");
      close(fd_shm_ps);
      fd_shm_ps = -1;
      return false;
   }

   ptr_shm_ps = mmap(NULL, size_ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_ps, 0);
   if (ptr_shm_ps == MAP_FAILED) {
      perror("mmap");
      ptr_shm_ps = NULL;
      close(fd_shm_ps);
      fd_shm_ps = -1;
      return false;
   }

   memset(ptr_shm_ps, 0, size_ps);
   shm_count = (size_t)n_pid;
   return true;
}

int attach_shm(const char *path_shm, size_t n_pid) {
   if (!path_shm || n_pid == 0) {
      errno = EINVAL;
      return false;
   }

   if (ptr_shm_ps) {
      return true;
   }

   size_t size_ps = n_pid * sizeof(struct ProcessState);

   fd_shm_ps = shm_open(path_shm, O_RDWR, 0666);
   if (fd_shm_ps == -1) {
      perror("shm_open attach");
      return false;
   }

   ptr_shm_ps = mmap(NULL, size_ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_ps, 0);
   if (ptr_shm_ps == MAP_FAILED) {
      perror("mmap attach");
      ptr_shm_ps = NULL;
      close(fd_shm_ps);
      fd_shm_ps = -1;
      return false;
   }

   shm_count = n_pid;
   return true;
}

void close_shm(size_t n_pid) {
   if (ptr_shm_ps) {
      munmap(ptr_shm_ps, n_pid * sizeof(struct ProcessState));
      ptr_shm_ps = NULL;
   }
   if (fd_shm_ps != -1) {
      close(fd_shm_ps);
      fd_shm_ps = -1;
   }
   shm_count = 0;
}

int init_pid_hash_map(pid_t *pids_process, size_t n_pid) {
   if (!pids_process) {
      return -1;
   }

   clear_map_pid();
   for (size_t i = 0; i < n_pid; i++) {
      if (!insert_map_pid(pids_process[i])) {
         return -1;
      }
   }
   return 0;
}

pid_t *start_all_process(struct ParsedData *data, const char *path_shm) {
   if (!data || data->exe.count_exe == 0 || !data->exe.path_exe) {
      errno = EINVAL;
      return NULL;
   }

   pid_t *pids_process = calloc(data->exe.count_exe, sizeof(pid_t));
   if (!pids_process) {
      perror("calloc pids_process");
      return NULL;
   }

   char idx_env[64];
   char count_env[64];

   for (size_t i = 0; i < data->exe.count_exe; i++) {
      pid_t child_pid = fork();
      if (child_pid < 0) {
         perror("fork");
         free(pids_process);
         return NULL;
      }

      if (child_pid == 0) {
         snprintf(idx_env, sizeof(idx_env), "%zu", i);
         snprintf(count_env, sizeof(count_env), "%zu", data->exe.count_exe);

         setenv("WATCHLION_SHM_NAME", path_shm ? path_shm : WATCHLION_DEFAULT_SHM_NAME, 1);
         setenv("WATCHLION_INDEX", idx_env, 1);
         setenv("WATCHLION_COUNT", count_env, 1);

         execl(data->exe.path_exe[i], data->exe.name_exe[i], (char *)NULL);
         perror("execl");
         _exit(127);
      }

      pids_process[i] = child_pid;
   }

   return pids_process;
}

void check_status(pid_t *pids_process, int n_pid) {
   if (!pids_process || !ptr_shm_ps || n_pid <= 0) {
      return;
   }

   for (int i = 0; i < n_pid; i++) {
      int idx_process = get_idx_map_pid(pids_process[i]);
      if (idx_process < 0 || (size_t)idx_process >= shm_count) {
         continue;
      }

      struct ProcessStatus *ps = malloc(sizeof(struct ProcessStatus));
      if (!ps) {
         return;
      }

      ps->pid_process = pids_process[i];

      if (ptr_shm_ps[idx_process].alive) {
         ps->warned = true;
         if (!enqueue_process_status(ps)) {
            free(ps);
         }
         printf("PID:%d\tVIVO\n", pids_process[i]);
         ptr_shm_ps[idx_process].alive = 0;
      } else {
         ps->warned = false;
         if (!enqueue_process_status(ps)) {
            free(ps);
         }
         printf("PID:%d\tMORTO\n", pids_process[i]);

         if (ptr_shm_ps[idx_process].callback_pd) {
            ptr_shm_ps[idx_process].callback_pd(ptr_shm_ps[idx_process].context);
         }
      }
   }
}

void set_status_alive(int idx_pid) {
   if (!ptr_shm_ps || idx_pid < 0 || (size_t)idx_pid >= shm_count) {
      return;
   }
   ptr_shm_ps[idx_pid].alive = 1;
}

void set_callback_post_death(void (* const callback)(void *), void *context, int idx_pid) {
   if (!ptr_shm_ps || idx_pid < 0 || (size_t)idx_pid >= shm_count) {
      return;
   }
   ptr_shm_ps[idx_pid].callback_pd = callback;
   ptr_shm_ps[idx_pid].context = context;
}