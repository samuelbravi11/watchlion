#ifndef PARSED_DATA_H
#define PARSED_DATA_H

#include <stddef.h>
#include "exe.h"
#include "log.h"
#include "type_log.h"

#define DEF_COUNT_STRUCTS 3 // almeno 3 struct sennò non funziona

struct ParsedData {
   struct Exe exe;
   struct Log log;
   struct TypeLog tp_log;
   size_t count;
};

#endif