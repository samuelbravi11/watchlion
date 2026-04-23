#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "../include/counter.h"
#include "../include/queue_sig.h"

// global variable for checking report in polling mode
static volatile bool report = 0;

static struct Elem *createElem(int pid, int mod) {
    struct Elem *e = malloc(sizeof (struct Elem));
    if (!e) {
        perror("Errore allocazione memoria");
        return 0;
    }
    e->pid = pid;
    e->mod = mod;
    return e;
}

// handler più corto possibile --> cattura informazioni e le incoda --> il main processa la coda
void handler(int signo, siginfo_t *info, void *empty) {
    switch (signo)
    {
    case SIGUSR1:
        enqueue_elem(createElem(info->si_pid, 1));
        break;
    case SIGUSR2:
        enqueue_elem(createElem(info->si_pid, 2));
        break;
    default: //SIGINT | SIGTERM
        report = 1;
        break;
    }
}

void startPolling(sigset_t *mask) {
    struct Elem *e;
    while (!report) {
        // svuoto coda elementi e li processo
        while ((e = dequeue_elem())) {
            updateCounter(e);
            free(e);
        }
        // aspetto solo i 4 signals
        sigsuspend(mask);
    }
}