#ifndef WATCHDOG_H
#define WATCHDOG_H

// setta il sigaction 
void setWatchlion(struct sigaction *sa, int signo);

// imposta 1 nell'array wtg di pid processi
int imAlive(int pid_watchlion, int pid_process);

#endif