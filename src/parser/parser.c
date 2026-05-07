#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "id_struct.h"
#include "parsed_data.h"
#include "parser.h"

#define MAX_LINE 1024

static FILE *cfg = NULL;

static char *xstrdup(const char *s) {
    if (!s) {
        return NULL;
    }

    size_t len = strlen(s) + 1;
    char *copy = malloc(len);

    if (!copy) {
        return NULL;
    }

    memcpy(copy, s, len);
    return copy;
}

static void init_parsed_data(struct ParsedData *pd) {
    memset(pd, 0, sizeof(*pd));

    pd->exe.name = NAME_EXE;
    pd->log.name = NAME_LOG;
    pd->tp_log.name = NAME_TYPE_LOG;
}

static char *trim_left(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') {
        s++;
    }

    return s;
}

static void trim_right(char *s) {
    size_t len = strlen(s);

    while (len > 0) {
        char c = s[len - 1];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            s[len - 1] = '\0';
            len--;
        } else {
            break;
        }
    }
}

static char *trim(char *s) {
    s = trim_left(s);
    trim_right(s);
    return s;
}

char *getNameExeFromPath(const char *s, size_t len) {
    (void)len;

    if (!s) {
        return NULL;
    }

    const char *last_slash = strrchr(s, '/');

    if (!last_slash) {
        return (char *)s;
    }

    return (char *)(last_slash + 1);
}

static int parse_exe_line(struct ParsedData *pd, char *values) {
    char *save_word = NULL;
    char *word = strtok_r(values, " \t\r\n", &save_word);

    while (word != NULL) {
        char **new_paths = realloc(
            pd->exe.path_exe,
            (pd->exe.count_exe + 1) * sizeof(char *)
        );

        if (!new_paths) {
            perror("realloc path_exe");
            return -1;
        }

        pd->exe.path_exe = new_paths;

        char **new_names = realloc(
            pd->exe.name_exe,
            (pd->exe.count_exe + 1) * sizeof(char *)
        );

        if (!new_names) {
            perror("realloc name_exe");
            return -1;
        }

        pd->exe.name_exe = new_names;

        pd->exe.path_exe[pd->exe.count_exe] = xstrdup(word);

        if (!pd->exe.path_exe[pd->exe.count_exe]) {
            perror("strdup path_exe");
            return -1;
        }

        char *name = getNameExeFromPath(word, strlen(word));
        pd->exe.name_exe[pd->exe.count_exe] = xstrdup(name);

        if (!pd->exe.name_exe[pd->exe.count_exe]) {
            perror("strdup name_exe");
            return -1;
        }

        pd->exe.count_exe++;

        word = strtok_r(NULL, " \t\r\n", &save_word);
    }

    return 0;
}

static int parse_log_line(struct ParsedData *pd, char *values) {
    char *save_word = NULL;

    char *path = strtok_r(values, " \t\r\n", &save_word);
    char *flag = strtok_r(NULL, " \t\r\n", &save_word);

    if (!path || !flag) {
        fprintf(stderr, "Errore LOG: formato corretto: LOG: path flag\n");
        return -1;
    }

    pd->log.path_file = xstrdup(path);
    pd->log.flag = xstrdup(flag);

    if (!pd->log.path_file || !pd->log.flag) {
        perror("strdup log");
        return -1;
    }

    return 0;
}

static int parse_type_log_line(struct ParsedData *pd, char *values) {
    char *save_word = NULL;
    char *word = strtok_r(values, " \t\r\n", &save_word);

    while (word != NULL) {
        if (strcmp(word, "signal") == 0) {
            pd->tp_log.sig = true;
        } else if (strcmp(word, "end_child") == 0) {
            pd->tp_log.end_child = true;
        } else {
            fprintf(stderr, "Errore TYPE_LOG: valore sconosciuto '%s'\n", word);
            return -1;
        }

        word = strtok_r(NULL, " \t\r\n", &save_word);
    }

    return 0;
}

static int parse_line(struct ParsedData *pd, char *line) {
    char *save_section = NULL;

    char *section = strtok_r(line, ":", &save_section);
    char *values = strtok_r(NULL, "", &save_section);

    if (!section) {
        return 0;
    }

    section = trim(section);

    if (section[0] == '\0') {
        return 0;
    }

    if (section[0] == '#') {
        return 0;
    }

    if (!values) {
        fprintf(stderr, "Errore: sezione '%s' senza valori\n", section);
        return -1;
    }

    values = trim(values);

    if (strcmp(section, "EXE") == 0 || strcmp(section, NAME_EXE) == 0) {
        return parse_exe_line(pd, values);
    }

    if (strcmp(section, "LOG") == 0 || strcmp(section, NAME_LOG) == 0) {
        return parse_log_line(pd, values);
    }

    if (strcmp(section, "TYPE_LOG") == 0 || strcmp(section, NAME_TYPE_LOG) == 0) {
        return parse_type_log_line(pd, values);
    }

    fprintf(stderr, "Errore: sezione sconosciuta '%s'\n", section);
    return -1;
}

int parse_file(int fd_i, struct ParsedData *pa) {
    if (fd_i < 0 || !pa) {
        return -1;
    }

    init_parsed_data(pa);

    int fd_copy = dup(fd_i);

    if (fd_copy == -1) {
        perror("dup config fd");
        return -1;
    }

    cfg = fdopen(fd_copy, "r");

    if (!cfg) {
        perror("fdopen config fd");
        close(fd_copy);
        return -1;
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), cfg) != NULL) {
        if (parse_line(pa, line) != 0) {
            fclose(cfg);
            cfg = NULL;
            return -1;
        }
    }

    fclose(cfg);
    cfg = NULL;

    return 0;
}