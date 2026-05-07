#ifndef PROCESS_STATUS_H
#define PROCESS_STATUS_H

#include <sys/types.h>
#include <stdbool.h>

struct ProcessStatus {
   pid_t pid_process;
   bool warned;
};

#endif