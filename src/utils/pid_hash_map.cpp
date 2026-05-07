#include <unordered_map>
#include "pid_hash_map.h"

static std::unordered_map<pid_t, int> map;
static int count_pid = 0;

bool insert_map_pid(pid_t pid) {
   if (!map.contains(pid)) {
      map[pid] = count_pid;
      count_pid++;
      return true;
   }
   return false;
}

int get_idx_map_pid(pid_t pid) {
   if (map.contains(pid)) {
      return map[pid];
   }
   return -1;
}

void clear_map_pid(void) {
   map.clear();
   count_pid = 0;
}