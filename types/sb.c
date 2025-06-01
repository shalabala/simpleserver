
#include <stdlib.h>

#include "../utility/error.h"
#include "sb.h"

static int get_capacity(size_t start, size_t initial_capacity) {
  while (start < initial_capacity) {
    start <<= 1; // Ensure capacity is a power of two
  }
  return start;
}

int sbinit(sb *str, size_t initial_capacity) {
  if (!str) {
    return RAISE_ARGERR(
        "null string buffer cannot be initialized"); // Memory allocation failed
  }
  size_t capacity = get_capacity(1, initial_capacity);
  str->data = calloc(sizeof(char), capacity);

  if (!str->data) {
    free(str);
    return RAISE_MALLOC(
        "could not allocate string buffer data"); // Memory allocation failed
  }

  str->size = 0;
  str->capacity = capacity;
  return 0;
}

sb *sbcreate(size_t initial_capacity) {
  sb *str = malloc(sizeof(sb));
  if (!str) {
    return NULL; // Memory allocation failed
  }
  if (sbinit(str, initial_capacity) != 0) {
    free(str);   // Free the struct if initialization fails
    return NULL; // Memory allocation failed
  }
  return str; // Return the initialized string buffer
}

int sbappend(sb *str, const char *data, size_t len) {
  if (!str || !data || len == 0) {
    return RAISE_ARGERR("null string buffer or data cannot be appended");
  }

  if (str->size + len + 1 >= str->capacity) {
    size_t new_capacity = get_capacity(str->capacity << 1, str->size + len + 1);
    char *new_data = realloc(str->data, new_capacity);
    if (!new_data) {
      return RAISE_MALLOC(
          "could not reallocate sb data to fit new size."); // Memory allocation
                                                            // failed
    }
    str->data = new_data;
    str->capacity = new_capacity;
  }
  memcpy(str->data + str->size, data, len);
  str->data[str->size + len] = 0; // Null-terminate the string
  str->size += len;
  return OK; // Success
}

int sbinsert(sb *str, size_t pos, const char *data, size_t len) {
  if (pos > str->size) {
    return RAISE_ARGERR(
        "cannot insert into position greater than size."); // Invalid position
  }
  if (str->size + len + 1 >= str->capacity) {
    size_t new_capacity = str->capacity * 2;
    while (str->size + len >= new_capacity) {
      new_capacity *= 2; // Double the capacity until it fits
    }
    char *new_data = realloc(str->data, new_capacity);
    if (!new_data) {
      return RAISE_MALLOC("could not extend data"); // Memory allocation failed
    }
    str->data = new_data;
    str->capacity = new_capacity;
  }
  memmove(str->data + pos + len,
          str->data + pos,
          str->size - pos + 1); // +1 to include null terminator
  memcpy(str->data + pos, data, len);
  str->size += len;
  return 0; // Success
}

void sbfree(sb *str) {
  if (str) {
    if (str->data) {
      free(str->data);
    }
  }
}

int sbclear(sb *str) {
  if (str && str->data) {
    memset(str->data, 0, str->capacity);
    str->size = 0;
    return OK;
  }else{
    return RAISE_ARGERR("cannot clear uninitialized or null string buffer");
  }
}