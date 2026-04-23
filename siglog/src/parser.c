#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define MAX_DIM_IN 256

static void (* parser)(struct ParsedData *pd);

char *getNameExeFromPath(const char *s, size_t len) {
    // temp/boh/aaa/ses/a.out --> len = 34
    int idx_name_exe = len-1;
    for ( ; s[idx_name_exe] != "/"; idx_name_exe--) ;
    // --> /a.out\0 --> a.out
    idx_name_exe++;
    return s+idx_name_exe;
}

// exe: /temp/test/a.out /boh/beh/b.out
int exeParser(struct ParsedData *pd) {
    char str[MAX_STR];
    int str_length = 0;
    struct Exe *e = malloc(sizeof (struct Exe));
    if (!e) {
        perror("Errore allocazione memoria");
        return 1;
    }
    e->name = NAME_EXE;
    scanf(%s, str);
    str_length = strlen(str);
    while (strcmp(str, ";") != 0) {
        e->count_exe++;
        realloc(e->count_exe, sizeof (MAX_DIM_IN));
        e->path_exe[e->count_exe-1] = malloc(str_length); 
        e->path_exe[e->count_exe-1] = str;
        e->name_exe = malloc(str_length);
        e->name_exe = getNameExeFromPath(str);
        scanf(%s, str);
    }
    if (strcmp(str, ";") != 0) {
        perror("Errore formato file config");
        exit(0);
    }
    pd->exe = e;
    return 0;
}

// log: /temp/log/log.txt write/append
int logParser(struct ParsedData *pd) {
    struct Log *l;
    char end;
    l = malloc(sizeof (struct Log));
    if (!l) {
        perror("Errore allocazione memoria");
        return 2;
    }
    l->name = NAME_LOG;
    scanf("%s %s %s", l->path_file, l->flag, end);
    if (end != ';') {
        perror("Errore formato file config");
        exit(0);
    }
    return 0;
}

// type_log: signal end_child
int typeLogParser(struct ParsedData *pd) {
    char str[MAX_STR];
    int str_length = 0;
    struct TypeLog *tl = malloc(sizeof (struct TypeLog));
    if (!tl) {
        perror("Errore allocazione memoria");
        return 3;
    }
    tl->name = NAME_TYPE_LOG;
    scanf(%s, str);
    while (strcmp(str, ";") != 0) {
        if (strcmp(str, "signal"))
            tl->sig = true;
        else if (strcmp(str, "end_child"))
            tl->end_child = true;
        scanf(%s, str);
    }
    pd->type_log = tl;
    return 0;
}



int (* f)(struct ParsedData *pd) handleParseByStructId(enum IdStruct id) {
    switch (id) {
    case 1:
        return exeParser;
    case 2:
        return logParser;
    case 3:
        return typeLogParser;
    }
}

static enum TypeLog getIdStruct(char *s) {
    if (strcmp(s, "NAME_EXE") == 0)
        return ID_EXE;
    if (strcmp(s, "NAME_LOG") == 0)
        return ID_LOG;
    if (strcmp(s, "NAME_TYPE_LOG") == 0)
        return ID_TYPE_LOG;
    return ERROR;
}


int parseFile(int fd_i, struct ParsedData *pa) {
    if (dup2(fd_i, stdin) == -1) {
        perror("Error reading file init");
        return 1;
    }
    // line --> log: 2
    // exe --> exe: kdsjd ksjd skdj
    char *name_struct;
    enum type_struct;
    while (scanf("%s", name_struct) > 0) {
        type_struct = getIdStruct(name_struct);
        parser = handle_parse_by_struct_id(type_struct);
        // if there is an error during parser return
        if (status = parser(pa))
            return status;
    }
    return 0;
}