#include <stdio.h>
#include <stdlib.h>
#include "../include/counter.h"
#include "../include/process.h"

struct Counter {
    struct Process *process;
    size_t size;
    size_t items;
};

static struct Counter *counter;

void init_counter(const size_t num_process) {
    counter = malloc(sizeof (struct Counter));
    if (!counter) {
        perror("Errore allocazione memoria");
        exit(1);
    }
    counter->process = calloc(num_process, sizeof (struct Process));
    if (!counter->process) {
        perror("Errore allocazione memoria array processi");
        exit(1);
    }
    counter->size = num_process;
}

int updateCounter(const struct Elem *e) {
    if (counter->items > counter->size) {
        perror("Errore gestione items counter");
        exit(1);
    }
    // controlla se il pid è presente nei counter
    for (int i = 0; i < (int)counter->size; i++) {
        // se presente aggiorna
        if (counter->process[i].pid == e->pid) {
            (e->mod == 1) ? counter->process[i].n_usr1++ : counter->process[i].n_usr2++;
            return 1;
        }
    }
    // se non trovato e counter pieno esci
    if (counter->items == counter->size) {
        perror("Counter pieno!");
        return 0;
    }
    int i = counter->items;
    counter->process[i].pid = e->pid;
    counter->process[i].n_usr1 = 0; counter->process[i].n_usr2 = 0;
    (e->mod == 1) ? counter->process[i].n_usr1++ : counter->process[i].n_usr2++;
    counter->items++;

    return 1;
}

void printReport(void) {
    for (int i = 0; i < (int)counter->items; i++) {
        printf("PID: %d\tN_SIGUSR1: %d\tN_SIGUSR2: %d\n",
            counter->process[i].pid, counter->process[i].n_usr1, counter->process[i].n_usr2);
    }
}