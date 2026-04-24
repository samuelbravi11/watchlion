#ifndef APP_H
#define APP_H

#include "parsed_data.h"

int initProvvisoriaMem(struct ParsedData *pd);
int startProcess(struct ParsedData *data, int wtd_pid);
int startSigCounter(char *path_exe_sigcounter, char *name_exe_sigcounter, int n_pid);

#endif