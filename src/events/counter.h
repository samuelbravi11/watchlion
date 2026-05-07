#ifndef COUNTER_H
#define COUNTER_H

#include <signal.h>
#include <stddef.h>
#include "process_status.h"

void report_handler(int signo);
void init_stats(const pid_t *pids_process, int num_pid);
void check_events(void);
void print_report(void);

#endif