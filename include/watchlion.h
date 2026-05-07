#ifndef WATCHLION_H
#define WATCHLION_H

#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {   // dicono al compilatore di buildare in C queste funzioni
#endif

void set_watchlion(struct sigaction *sa,
                   void (* const user_handler)(int, siginfo_t *, void *),
                   int signo,
                   bool mod);

bool im_alive(pid_t pid_process);

bool set_behave_post_death(void (* const callback_post_death)(void *),
                           void *context,
                           pid_t pid_process);

#ifdef __cplusplus
}
#endif

#endif