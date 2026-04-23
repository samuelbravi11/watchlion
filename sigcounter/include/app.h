#ifndef APP_H
#define APP_H

#include <signal.h>

void startPolling(sigset_t *mask);
void handler(int signo, siginfo_t *info, void *empty);

#endif