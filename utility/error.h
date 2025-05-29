#ifndef ERROR_H
#define ERROR_H

#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define ERRLENGTH 256
typedef enum _severity { WARNING, ERROR, CRITICAL } severity;

typedef enum _errcode { MALLOC = 1, ARG, BUFFER_LIMIT, RECEIVE_FAILED } errcode;

typedef struct _error {
  char description[ERRLENGTH];
  const severity severity;
  const errcode code;

} error;

extern error malloc_err;
extern error arg_err;
extern error bufflimit_reached;
extern error receive_failed;
extern error *globerr;

/**
 * Signifies that an error has occurred. Returns the error code.
 * The error message is formatted using the provided format string and arguments.
 * @param c The format string for the error message.
 * @param e Pointer to the error structure.
 * @param ... Additional arguments to format the error message.
 */
int raise(char* c, error *e, ...);

/**
 * Checks if there is an error currently stored in the global error variable.
 * @return true if there is an error, false otherwise.
 */
bool haserr();

/**
 * Retrieves the current error and clears the global error variable.
 * @return The current error structure.
 */
error* geterr();

#define RAISE_MALLOC(msg, ...) (raise(msg, &malloc_err, ##__VA_ARGS__))
#define RAISE_ARGERR(msg, ...) (raise(msg, &arg_err, ##__VA_ARGS__))
#define RAISE_RECVFAIL(msg, ...) (raise(msg, &receive_failed, ##__VA_ARGS__))
#define RAISE_BUFFLIMIT(msg, ...) (raise(msg, &bufflimit_reached, ##__VA_ARGS__))
#define OK 0
#endif // ERROR_H