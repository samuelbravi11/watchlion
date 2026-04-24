#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include "app.h"
#include "./include/queue_sig.h"
#include "./include/counter.h"
#include "./include/app.h"

int fd_log;

// inizializzazione sigation per signal custom
void init_sigaction(struct sigaction *sa) {
    sa->sa_sigaction = handler;
    sa->sa_flags = SA_SIGINFO | SA_RESTART;

    sigfillset(&sa->sa_mask);
    sigdelset(&sa->sa_mask, SIGUSR1);
    sigdelset(&sa->sa_mask, SIGUSR2);
    sigdelset(&sa->sa_mask, SIGINT);
    sigdelset(&sa->sa_mask, SIGTERM);

    sigaction(SIGUSR1, sa, NULL);
    sigaction(SIGUSR2, sa, NULL);
    sigaction(SIGINT,  sa, NULL);
    sigaction(SIGTERM, sa, NULL);
}

// creo mask base dove aspetto solo i 4 signals
void set_mask(sigset_t *mask) {
    sigfillset(mask);
    sigdelset(mask, SIGUSR1);
    sigdelset(mask, SIGUSR2);
    sigdelset(mask, SIGINT);
    sigdelset(mask, SIGTERM);
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sigset_t mask_base;

    // .out + num_counter + path_logfile
    if (argc != 3) {
        perror("Formato: eseguibile + num mittenti");
        puts("Esempio: ./a.out 5 /home/usr/filelog.txt");
        exit(2);
    }

    // init
    fd_log = open(argv[2], O_CREAT | O_WRITE);
    init_sigaction(&sa);
    queue_init();
    init_counter(atoi(argv[1]));
    set_mask(&mask_base);

    // main logic
    printf("PID: %d\n", getpid());
    startPolling(&mask_base);
    printReport();
    // freeALLLLL();    TODO

    return 0;
}
