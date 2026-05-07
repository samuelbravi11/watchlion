#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "parsed_data.h"
#include "id_struct.h"

char *getNameExeFromPath(const char *s, size_t len);
int exeParser(struct ParsedData *pd);
int logParser(struct ParsedData *pd);
int typeLogParser(struct ParsedData *pd);
int handle_parse_by_struct_id(enum IdStruct id);
int parse_file(int fd_i, struct ParsedData *pa);

#endif