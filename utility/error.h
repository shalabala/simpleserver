#ifndef ERROR_H
#define ERROR_H

#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


#define ERRLENGTH 256
typedef enum _severity { WARNING, ERROR, CRITICAL } severity;

typedef enum _errcode { MALLOC = 1, ARG } errcode;

typedef struct _error {
  char description[ERRLENGTH];
  const severity severity;
  const errcode code;

} error;

error malloc_err = {"", CRITICAL, MALLOC};
error arg_err = {"", ERROR, ARG };
error *globerr;

int raise(char* c, error *e, ...){
    va_list args;
    va_start(args, e);

    globerr = e;

    snprintf(globerr->description, ERRLENGTH, c, args );

    return (int)globerr->code;
    va_end(args);
}

bool haserr(){
    return globerr != NULL;
}

error geterr(){
    error e =*globerr;
    globerr=NULL;
    return e;


}

#define RAISE_MALLOC(msg, ...) (raise(msg, &malloc_err, ##__VA_ARGS__))
#define RAISE_ARGERR(msg, ...) (raise(msg, &arg_err, ##__VA_ARGS__))


#endif // ERROR_H