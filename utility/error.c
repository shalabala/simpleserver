#include "error.h"
#include <stdlib.h>

error malloc_err = {"", CRITICAL, MALLOC};
error arg_err = {"", ERROR, ARG};
error bufflimit_reached = {"", ERROR, BUFFER_LIMIT};
error receive_failed = {"", ERROR, RECEIVE_FAILED};
error unsupported_method = {"", ERROR, UNSUPPORTED_METHOD};
error malformed_request = {"", ERROR, MALFORMED_REQ};
error cmap_full = {"", FATAL, CMAP_FULL};
error invalid_cpath = {"", FATAL, INVALID_CONTROLLER_PATH};
error invalid_template = {"", ERROR, INVALID_TEMPLATE};
error notfound = {"", WARNING, NOTFOUND};
error *globerr = NULL;

handler lvl_handler[NO_OF_ERRORLEVELS] = {0};
handler error_handler[NO_OF_ERRORS] = {0};

int raise(char *c, error *e, ...) {

  va_list args;
  va_start(args, e);

  globerr = e;

  snprintf(globerr->description, ERRLENGTH, c, args);

  va_end(args);

  if (error_handler[globerr->code]) {
    error_handler[globerr->code]();
  } else if (lvl_handler[globerr->severity]) {
    lvl_handler[globerr->severity]();
  } else if (globerr->severity == FATAL) {
    fatal_error_handler();
  }

  return (int)globerr->code;
}

bool haserr() { return globerr != NULL; }

error *geterr() {
  error *e = globerr;
  globerr = NULL;
  return e;
}

void cleargloberr() { globerr = NULL; }

void reg_err_level_handler(severity severity, handler handler) {
  if (severity < 0 || severity > NO_OF_ERRORLEVELS) {
    fprintf(stderr,
            "Cannot register handler to level %d which is greater"
            "than the maximum allowed number of handlers. Consider raising the "
            "NO_OF_ERRORLEVELS constant",
            (int)severity);
  }
  lvl_handler[severity] = handler;
}

void reg_err_kind_handler(errcode code, handler handler) {
  if (code < 0 || code > NO_OF_ERRORS) {
    fprintf(stderr,
            "Cannot register handler to level %d which is greater"
            "than the maximum allowed number of handlers. Consider raising the "
            "NO_OF_ERRORLEVELS constant",
            (int)code);
  }
  lvl_handler[code] = handler;
}

void fatal_error_handler() {
  fprintf(stderr, "%s", globerr->description);
  abort();
}
