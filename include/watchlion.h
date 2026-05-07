#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <signal.h>
#include <stdbool.h>

// setta il sigaction 
void set_watchlion(struct sigaction *sa, void (* const user_handler)(int, siginfo_t *, void *), int signo, bool mod);

// imposta 1 nell'array wtg di pid processi
bool im_alive(int pids_process);

bool set_behave_post_death(void (* const callback_post_death)(void *), void *context, int pids_process);

#endif