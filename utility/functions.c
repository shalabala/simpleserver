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

bool isalphabetical(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isnumeric(char c) { return (c >= '0' && c <= '9'); }

bool isalphanumeric(char c) { return isalphabetical(c) || isnumeric(c); }

size_t strncount(char *str, char c, size_t n) {
  size_t count = 0;
  for (size_t i = 0; i < n; ++i) {
    if (str[i] == 0) {
      break;
    }
    if(str[i] == c){
      ++count;
    }
  }
  return count;
}
