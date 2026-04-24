#ifndef PARSED_DATA_H
#define PARSED_DATA_H

#define DEF_COUNT_STRUCTS 3 // almeno 3 struct sennò non funziona

struct ParsedData {
   struct Exe *exe;
   struct Log *log;
   struct TypeLog *type_log;
   size_t count;
};

#endif