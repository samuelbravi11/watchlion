#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "counter.h"
#include "queue_sig.h"
#include "../utils/pid_hash_map.h"

struct ProcessStats {
    pid_t pid_process;
    int reply_count;
    int noreply_count;
};

static struct ProcessStats *p_stats;
static int n_pid;

// global variable for checking report in polling mode
static volatile bool report = 0;


// handler più corto possibile --> cattura informazioni e le incoda --> il main processa la coda
void report_handler(int signo) {
    report = 1;
}

void init_stats(const pid_t *pids_process, int num_pid) {
    n_pid = num_pid;
    p_stats = calloc(n_pid, sizeof (struct ProcessStats));
    if (!p_stats) {
        perror("Errore allocazione memoria");
        exit(1);
    }
    for (int i = 0; i < n_pid; i++) {
        p_stats[i].pid_process = pids_process[i];
        p_stats[i].reply_count = 0;
        p_stats[i].noreply_count = 0;
    }
}

static void update_stats(const struct ProcessStatus *ps) {
    int idx_process = get_idx_map_pid(ps->pid_process);
    ps->warned ? p_stats[idx_process].reply_count++ : p_stats[idx_process].noreply_count++;
}

void check_events(void) {
    if (report) {
        report = 0;
        print_report();
    }
    struct ProcessStatus *ps;
    // svuoto coda elementi e li processo
    while ((ps = dequeue_process_status())) {
        update_stats(ps);
        free(ps);
    }
}

void print_report(void) {
    for (int i = 0; i < n_pid; i++) {
        printf("PID: %d\tREPLY: %d\tNO-REPLY: %d\n",
            p_stats[i].pid_process, p_stats[i].reply_count, p_stats[i].noreply_count);
    }
}