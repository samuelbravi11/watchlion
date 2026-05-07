#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "parsed_data.h"

char *getNameExeFromPath(const char *s, size_t len);
int parse_file(int fd_i, struct ParsedData *pa);

#endif