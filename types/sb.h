#ifndef SB_H
#define SB_H
#include <string.h>

/**
 * A simple string buffer structure to hold dynamic strings.
 */
typedef struct _sb {
  char *data;
  size_t size;
  size_t capacity;
} sb;

/**
 * Creates a string buffer with a specified initial capacity.
 *
 * @param str Pointer to the string buffer to initialize.
 * @param initial_capacity The initial capacity of the string buffer.
 * @return A pointer to the initialized instance on success, or NULL on failure
 * (e.g., memory allocation error).
 */
sb *sbcreate(size_t initial_capacity);

/**
 * Initializes a string buffer with a specified initial capacity.
 *
 * @param initial_capacity The initial capacity of the string buffer.
 * @return 0 on success, or -1 on failure (e.g., memory allocation error).
 */
int sbinit(sb *str, size_t initial_capacity);

/**
 * Appends a string to the string buffer.
 *
 * @param str Pointer to the string buffer.
 * @param append The string to append.
 * @return 0 on success, or -1 on failure (e.g., memory allocation error).
 */
int sbappend(sb *str, const char *data, size_t len);

/**
 * Inserts a string into the string buffer at a specified position.
 *
 * @param str Pointer to the string buffer.
 * @param pos The position at which to insert the string.
 * @param data The string to insert.
 * @param len The length of the string to insert.
 * @return 0 on success, or -1 on failure (e.g., memory allocation error).
 */
int sbinsert(sb *str, size_t pos, const char *data, size_t len);

/**
 * Frees the memory allocated for the string buffer.
 *
 * @param str Pointer to the string buffer to free.
 */
void sbfree(sb *str);

/**
 * Cleares the string buffer. Does not reduce capacity.
 * @return 0 on success, error code otherwise
 */
int sbclear(sb *str);
#endif // SB_H
