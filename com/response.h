#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdint.h>

typedef struct _responsecode {
  uint16_t code;
  char *name;
  size_t namelen;
} responsecode;

typedef struct _response {
  responsecode resp_c;
  smap header;
  char *body;
  size_t bodylen;

} response;

int respinit(response *resp);

int respsend(response *resp, int socket);

void respfree(response *resp);


#endif // RESPONSE_H