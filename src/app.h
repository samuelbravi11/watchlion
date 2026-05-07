#ifndef APP_H
#define APP_H

#include "parsed_data.h"

int init_shm(const char *path_shm, int n_pid);
int init_pid_hash_map(pid_t *pids_process, size_t n_pid);
pid_t *start_all_process(struct ParsedData *data);
void check_status(pid_t *pids_process, int n_pid);
void set_status_alive(int idx_pid);
void set_callback_post_death(void (* const callback)(void *), void *context, int idx_pid);

#endif