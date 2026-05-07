#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool insert_map_pid(pid_t pid);
int get_idx_map_pid(pid_t pid);
void clear_map_pid(void);

#ifdef __cplusplus
}
#endif

#endif