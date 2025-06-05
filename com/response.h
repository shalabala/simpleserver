#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdint.h>
#include "../types/smap.h"
#include "../types/sb.h"

typedef struct _responsecode {
  uint16_t code;
  char name[16];
  size_t namelen;
} responsecode;

typedef struct _response {
  responsecode respcode;
  smap header;
  sb body;
} response;

int respinit(response *resp, uint16_t respcode, char *respname);

int respsend(response *resp, int socket);

int create_html(response *resp, const char *file, smap *context);

void respfree(response *resp);

int redirect(response *resp, char *whereto);

int set_resp_code(response *resp, char *name, uint16_t code);

#endif // RESPONSE_H