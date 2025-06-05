#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ERRLENGTH 256
typedef enum _severity { WARNING, ERROR, CRITICAL, FATAL } severity;

typedef enum _errcode {
  MALLOC = 1,
  ARG,
  BUFFER_LIMIT,
  RECEIVE_FAILED,
  UNSUPPORTED_METHOD,
  MALFORMED_REQ,
  CMAP_FULL,
  INVALID_CONTROLLER_PATH,
  INVALID_TEMPLATE,
  NOTFOUND
} errcode;

typedef struct _error {
  char description[ERRLENGTH];
  const severity severity;
  const errcode code;

} error;

extern error malloc_err;
extern error arg_err;
extern error bufflimit_reached;
extern error receive_failed;
extern error unsupported_method;
extern error malformed_request;
extern error cmap_full;
extern error invalid_cpath;
extern error invalid_template;
extern error notfound;
extern error *globerr;

/**
 * Signifies that an error has occurred. Returns the error code.
 * The error message is formatted using the provided format string and
 * arguments.
 * @param c The format string for the error message.
 * @param e Pointer to the error structure.
 * @param ... Additional arguments to format the error message.
 */
int raise(char *c, error *e, ...);

/**
 * Checks if there is an error currently stored in the global error variable.
 * @return true if there is an error, false otherwise.
 */
bool haserr();

/**
 * Retrieves the current error and clears the global error variable.
 * @return The current error structure.
 */
error *geterr();

void cleargloberr();

#define RAISE_MALLOC(msg, ...) (raise(msg, &malloc_err, ##__VA_ARGS__))
#define RAISE_ARGERR(msg, ...) (raise(msg, &arg_err, ##__VA_ARGS__))
#define RAISE_RECVFAIL(msg, ...) (raise(msg, &receive_failed, ##__VA_ARGS__))
#define RAISE_BUFFLIMIT(msg, ...)                                              \
  (raise(msg, &bufflimit_reached, ##__VA_ARGS__))
#define RAISE_UNSPMETH(msg, ...)                                               \
  (raise(msg, &unsupported_method, ##__VA_ARGS__))
#define RAISE_MALFORMEDREQ(msg, ...) (raise(msg, &receive_failed, ##__VA_ARGS__))
#define RAISE_CMAPFULL(msg, ...) (raise(msg, &cmap_full, ##__VA_ARGS__))
#define RAISE_INVALIDCPATH(msg, ...) (raise(msg, &invalid_cpath, ##__VA_ARGS__))
#define RAISE_INVALIDTEMPLATE(msg, ...) (raise(msg, &invalid_template, ##__VA_ARGS__))
#define RAISE_NOTFOUND(msg, ...) (raise(msg, &notfound, ##__VA_ARGS__))
#define OK 0
#endif // ERROR_H