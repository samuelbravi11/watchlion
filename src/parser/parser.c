#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "id_struct.h"
#include "parsed_data.h"

#define MAX_DIM_IN 256

static int (* parser)(struct ParsedData *pd);

char *getNameExeFromPath(const char *s, size_t len) {
    // temp/boh/aaa/ses/a.out --> len = 34
    int idx_name_exe = len-1;
    for ( ; s[idx_name_exe] != '/'; idx_name_exe--) ;
    // --> /a.out\0 --> a.out
    idx_name_exe++;
    return s+idx_name_exe;
}

// exe: /temp/test/a.out /boh/beh/b.out
int exeParser(struct ParsedData *pd) {
    char str[MAX_DIM_IN];
    int str_length = 0;
    pd->exe.name = NAME_EXE;
    scanf("%s", str);
    str_length = strlen(str);
    while (strcmp(str, ";") != 0) {
        pd->exe.count_exe++;
        realloc(&pd->exe.path_exe, pd->exe.count_exe * sizeof (MAX_DIM_IN));
        pd->exe.path_exe[pd->exe.count_exe-1] = malloc(str_length); 
        pd->exe.path_exe[pd->exe.count_exe-1] = str;    // TODO strdup
        pd->exe.name_exe = malloc(str_length);
        pd->exe.name_exe[pd->exe.count_exe - 1] = getNameExeFromPath(str, strlen(str));
        scanf("%s", str);
    }
    if (strcmp(str, ";") != 0) {
        perror("Errore formato file config");
        exit(0);
    }
    return 0;
}

// log: /temp/log/log.txt write/append
int logParser(struct ParsedData *pd) {
    char end; 
    pd->log.name = NAME_LOG;
    scanf("%s %s %s", pd->log.path_file, pd->log.flag, end);
    if (end != ';') {
        perror("Errore formato file config");
        exit(0);
    }
    return 0;
}

// type_log: signal end_child
int typeLogParser(struct ParsedData *pd) {
    char str[MAX_DIM_IN];
    int str_length = 0;
    
    pd->tp_log.name = NAME_TYPE_LOG;
    scanf("%s", str);
    while (strcmp(str, ";") != 0) {
        if (strcmp(str, "signal"))
            pd->tp_log.sig = true;
        else if (strcmp(str, "end_child"))
            pd->tp_log.end_child = true;
        scanf("%s", str);
    }
    return 0;
}

int handle_parse_by_struct_id(enum IdStruct id) {
    switch (id) {
    case 1:
        parser = exeParser;
        break;
    case 2:
        parser = logParser;
        break;
    case 3:
        parser = typeLogParser;
        break;
    default:
        return 0;
    }
    return id;
}

static enum IdStruct getIdStruct(char *s) {
    if (strcmp(s, "NAME_EXE") == 0)
        return ID_EXE;
    if (strcmp(s, "NAME_LOG") == 0)
        return ID_LOG;
    if (strcmp(s, "NAME_TYPE_LOG") == 0)
        return ID_TYPE_LOG;
    return ID_ERROR;
}


int parse_file(int fd_i, struct ParsedData *pa) {
    if (dup2(fd_i, stdin) == -1) {
        perror("Error reading file init");
        return 1;
    }
    char *name_struct;
    enum IdStruct id;
    int status;
    while (scanf("%s", name_struct) > 0) {
        id = getIdStruct(name_struct);
        if (!handle_parse_by_struct_id(id))
            return -1;
        // if there is an error during parser return
        if (status = parser(pa))
            return status;
    }
    return 0;
}