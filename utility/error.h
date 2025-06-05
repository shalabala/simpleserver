#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ERRLENGTH 256
#define NO_OF_ERRORLEVELS 4
#define NO_OF_ERRORS 100

typedef enum _severity { WARNING, ERROR, CRITICAL, FATAL } severity;

typedef void (*handler)();

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
  ITEM_NOTFOUND,
  FILE_NOTFOUND,
  INVALID_CONTROLLER_CONFIG
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
extern error item_notfound;
extern error file_notfound;
extern error invalid_ctrl_config;
extern error *globerr;

/**
 * Severity level based handlers.
 */
extern handler lvl_handler[NO_OF_ERRORLEVELS];

/**
 * Error code based handlers.
 */
extern handler error_handler[NO_OF_ERRORS];

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
 * Retrieves the current error code.
 */
int geterrcode();

/**
 * Register an error handler based on the errors severity.
 */
void reg_err_level_handler(severity severity, handler handler);

/**
 * Register an error handler based on the error code.
 */
void reg_err_kind_handler(errcode code, handler handler);

/**
 * Prints error message and then aborts the program. Default error handler for
 * fatal errors.
 */
void fatal_error_handler();

void cleargloberr();

#define RAISE_MALLOC(msg, ...) (raise(msg, &malloc_err, ##__VA_ARGS__))
#define RAISE_ARGERR(msg, ...) (raise(msg, &arg_err, ##__VA_ARGS__))
#define RAISE_RECVFAIL(msg, ...) (raise(msg, &receive_failed, ##__VA_ARGS__))
#define RAISE_BUFFLIMIT(msg, ...)                                              \
  (raise(msg, &bufflimit_reached, ##__VA_ARGS__))
#define RAISE_UNSPMETH(msg, ...)                                               \
  (raise(msg, &unsupported_method, ##__VA_ARGS__))
#define RAISE_MALFORMEDREQ(msg, ...)                                           \
  (raise(msg, &receive_failed, ##__VA_ARGS__))
#define RAISE_CMAPFULL(msg, ...) (raise(msg, &cmap_full, ##__VA_ARGS__))
#define RAISE_INVALIDCPATH(msg, ...) (raise(msg, &invalid_cpath, ##__VA_ARGS__))
#define RAISE_INVALIDTEMPLATE(msg, ...)                                        \
  (raise(msg, &invalid_template, ##__VA_ARGS__))
#define RAISE_ITEM_NOTFOUND(msg, ...) (raise(msg, &item_notfound, ##__VA_ARGS__))
#define RAISE_INVALID_CTRLCONF(msg, ...) (raise(msg, &invalid_ctrl_config, ##__VA_ARGS__))
#define RAISE_FILE_NOTFOUND(msg, ...) (raise(msg, &file_notfound, ##__VA_ARGS__))
#define OK 0
#endif // ERROR_H