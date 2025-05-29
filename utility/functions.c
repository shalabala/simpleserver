#include <stdbool.h>
#include <string.h>

#include "functions.h"
bool strneq(const char *key1,
            size_t key1len,
            const char *key2,
            size_t key2len) {
  if (key1len != key2len) {
    return false;
  }

  int cmp = strncmp(key1, key2, key1len);
  return cmp == 0;
}