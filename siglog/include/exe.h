#ifndef EXE_H
#define EXE_H

#include <stddef.h>

#define NAME_EXE "exe:"

struct Exe {
   char *name;
   char **path_exe;
   char **name_exe;
   size_t count_exe;
};

#endif