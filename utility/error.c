#include "error.h"
#include <stdlib.h>

error malloc_err = {"", CRITICAL, MALLOC};
error arg_err = {"", ERROR, ARG};
error bufflimit_reached = {"", ERROR, BUFFER_LIMIT};
error receive_failed = {"", ERROR, RECEIVE_FAILED};
error *globerr;

int raise(char *c, error *e, ...) {
  va_list args;
  va_start(args, e);

  globerr = e;

  snprintf(globerr->description, ERRLENGTH, c, args);

  va_end(args);

  return (int)globerr->code;
}

bool haserr() { return globerr != NULL; }

error *geterr() {
  error *e = globerr;
  globerr = NULL;
  return e;
}