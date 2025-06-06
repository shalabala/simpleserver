#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdint.h>
#include "../types/smap.h"
#include "../types/sb.h"
#define RESP_CODE_NAME_LEN 16
typedef struct _responsecode {
  uint16_t code;
  char name[RESP_CODE_NAME_LEN];
  size_t namelen;
} responsecode;

typedef struct _response {
  responsecode respcode;
  smap header;
  sb body;
} response;

int respinit(response *resp);

int respclear(response *resp);

int respsend(response *resp, int socket);

int create_html(response *resp, char *file, smap *context);

void respfree(response *resp);

int redirect(response *resp, char *whereto);

void set_resp_code(response *resp, char *name, uint16_t code);

int respprint(response *resp);

#endif // RESPONSE_H