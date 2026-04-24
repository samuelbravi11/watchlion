#ifndef TYPE_LOG_H
#define TYPE_LOG_H

#include <stdbool.h>

#define NAME_TYPE_LOG "type_log:"

struct TypeLog {
   char *name;
   bool sig;
   bool end_child;
};

#endif