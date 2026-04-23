#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>

struct Process {
   pid_t pid;
   int n_usr1;
   int n_usr2;
};

#endif